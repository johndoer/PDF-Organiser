#include "utils.h"
#include "treeview.h"
#include "window_gui.h"
#include "loaddata.h"
#include "context_menu.h"
#include "main.h"


/// Draw treeview
void make_treeview () {

	/// Column 1 - Pixbuf & Text combined column.
	GtkCellRenderer * pb = gtk_cell_renderer_pixbuf_new ();
	GtkCellRenderer * tx = gtk_cell_renderer_text_new ();
	gtk_cell_renderer_set_alignment (tx, 0.0, 0.5);
	GtkTreeViewColumn * col_pbtx = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_resizable (col_pbtx, TRUE);
	gtk_tree_view_column_set_title (col_pbtx,  "Name");
	gtk_tree_view_column_set_spacing (col_pbtx, 2);
	gtk_tree_view_column_set_expand (col_pbtx, TRUE);
	//--
	gtk_tree_view_column_pack_start (col_pbtx, pb, FALSE);
	gtk_tree_view_column_pack_start (col_pbtx, tx, TRUE);
	g_signal_connect (tx, "edited", (GCallback) cell_edited_callback, GTK_TREE_MODEL(treestore));
	gtk_tree_view_column_set_cell_data_func (col_pbtx, pb, set_tree_cell_pixbuf,  NULL, NULL);
	gtk_tree_view_column_set_cell_data_func (col_pbtx, tx, set_tree_cell_text,  NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), col_pbtx);

	/// Column 2 - filesize.
	GtkCellRenderer * sz = gtk_cell_renderer_text_new ();
	GtkTreeViewColumn * col_sz = gtk_tree_view_column_new_with_attributes ("Size",  sz, "text", 3,   NULL);
	gtk_tree_view_column_set_cell_data_func (col_sz, sz, filesize_Kib,  NULL, NULL);
	gtk_tree_view_column_set_resizable (col_sz, TRUE);
	gtk_cell_renderer_set_alignment (sz, 1.0, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), col_sz);

	gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW(treeview)), GTK_SELECTION_MULTIPLE);
	gtk_tree_view_set_reorderable ( GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_activate_on_single_click (GTK_TREE_VIEW (treeview), FALSE);

	/// Events
	g_signal_connect (GTK_TREE_MODEL(treestore), "row-inserted", G_CALLBACK(on_drag_data_inserted), NULL);
	g_signal_connect (GTK_TREE_MODEL(treestore), "row-changed", G_CALLBACK(on_row_changed), NULL);
	g_signal_connect (GTK_TREE_VIEW(treeview), "row-activated", G_CALLBACK(on_dblclick_row), NULL);
	g_signal_connect (GTK_TREE_VIEW(treeview), "button-press-event", (GCallback) treeview_click, NULL);

	/// Populate right click context menu
	table = g_hash_table_new(g_str_hash, g_str_equal);
	populate_context_menu_items();

	gtk_container_add(GTK_CONTAINER(sw), treeview);
}

void set_tree_cell_text (GtkTreeViewColumn *tc, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {
  gchar ** text;
  int folderNo;
  gtk_tree_model_get(model, iter, 1, &text, 5, &folderNo, -1);

	gboolean editable = (folderNo > 0) ? TRUE : FALSE;
	g_object_set (cell, "editable", editable, NULL);
  g_object_set (cell, "text", text, NULL);

  g_free(text);
}

void set_tree_cell_pixbuf (GtkTreeViewColumn *tc, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {
  int folderNo;
  gtk_tree_model_get (model, iter,   5, &folderNo,   -1);

	const gchar * file = (folderNo > 0) ? "folder" : "text-x-generic";
	GError *error = NULL;
	GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();
  GdkPixbuf *icon = gtk_icon_theme_load_icon (icon_theme, file, 32, GTK_ICON_LOOKUP_USE_BUILTIN, &error);

  g_object_set (cell, "pixbuf", icon, NULL);
}

void filesize_Kib (GtkTreeViewColumn *tc, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {

	/// Human readable filesizes

  int fileSize, folderNo;
  gtk_tree_model_get (model, iter, 3, &fileSize,   5, &folderNo,   -1);

  int n = gtk_tree_model_iter_n_children (model, iter);
  gchar * items = g_strdup_printf ("%i", n);
  items = g_strconcat (items, (n == 1) ? " item" : " items", NULL);

  gchar * text = g_format_size (fileSize);
  (folderNo > 0) ? g_object_set (cell, "text", items, NULL) : g_object_set (cell, "text", text, NULL);

  g_free(text);
  g_free(items);
}


/// Treeview events
void on_dblclick_row (GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata) {

	/// Launch pdf reader to view the pdf

	GtkTreeIter iter;
	GtkTreeModel * tree_model = gtk_tree_view_get_model (view);
	gtk_tree_model_get_iter (tree_model, &iter, path);

	int  id;
	gchar *fileName, *filePath;
	gtk_tree_model_get (tree_model, &iter,  0, &id,   1, &fileName,   2, &filePath,    -1);
	const gchar * fullpath = g_strconcat( filePath, G_DIR_SEPARATOR_S, fileName,  NULL);

	g_free (fileName);
	g_free (filePath);

	/// Check that file exists
	if ( ! g_file_test (fullpath, G_FILE_TEST_EXISTS)){
		gchar *value = g_strconcat("File '", fullpath, "' could not be found.", NULL);
		printf ("%s\n", value);
	  gtk_statusbar_push (GTK_STATUSBAR( statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR( statusbar), value), value);
	  g_free (value);
		return;
	}

	/// Open pdf file with pdf reader
  launch_pdfreader (fullpath);
}

void cell_edited_callback (GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer user_data) {

	/// Edit name of folders

  GtkTreeModel * model = GTK_TREE_MODEL (user_data);
  GtkTreeIter iter;
  gtk_tree_model_get_iter_from_string(model, &iter, path_string);

	/// get the id field of entry (based on iter) that has just been edited
	int  id;
	gtk_tree_model_get (model, &iter,  0, &id,  -1);

	/// find the array element and model element (based on id) this change refers to and update it.
	for (uint i = 0; i < PDFObjectsArray->len; i++)	{
		PDFObject * array_row = g_array_index(PDFObjectsArray, PDFObject*, i);
		if(array_row->id == id){
			array_row->fileName = new_text;
			// change has to also be applied to treemodel
			gtk_tree_store_set(GTK_TREE_STORE (model), &iter,   1, new_text, -1);
			break;
		}
	}

	/// change has to also be applied to object
	JsonNode * n = json_object_get_member (jo,  g_strdup_printf ("%i", id));
	JsonObject * o = json_node_get_object (n);
	json_object_set_string_member (o, "fileName", new_text);

	/// add the added-to object to a 'master' object
	JsonNode *no = json_node_alloc();
	json_node_init_object (no, o);
	json_object_set_member (jo, g_strdup_printf ("%i", id), no);

	make_text_red (status_changed);
	gtk_widget_set_sensitive (status_changed, TRUE);
	return;
}

void on_drag_data_inserted (GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data) {
	return;
}

void on_row_changed (GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data) {

	/// We need to determine here if triggerer is a 'dropped' element or 'dropped-to' element
	/// Dropped-to's event gets fired because you incremented its childs no
	/// On dropped a row into a folder (Other changes to row may also trigger this function)

	int  id; /// Droppee
	gtk_tree_model_get (tree_model, iter,  0, &id,  -1);


	/// Preserve orginal path
	gchar *path0 = gtk_tree_path_to_string (path);

	/// Get id of parent (dropped-to)
	/// Make path now point to parent (dropped-to)
	gtk_tree_path_up (path);
	int  id_parent;
	if (gtk_tree_path_get_depth (path) == 0){
		/// A parentDir isnt available - set it to 0
		id_parent = 0;
	} else {
		GtkTreeIter iterParent;
		gtk_tree_model_get_iter (tree_model, &iterParent, path);
		gtk_tree_model_get (tree_model, &iterParent,  0, &id_parent,  -1);
	}

	/// Apply change to jo, no need to bother to change PDFObjectsArray
	JsonNode * n = json_object_get_member (jo,  g_strdup_printf ("%i", id));

	if (!n){ return;}

	JsonObject * o = json_node_get_object (n);
	json_object_set_int_member (o, "parentDir", id_parent);

	if (! gtk_tree_path_to_string (path)){
		json_object_set_string_member (o, "position", path0);
	} else {
		json_object_set_string_member (o, "position", gtk_tree_path_to_string (path));
	}
	g_free(path0);

	/// Add the added-to object to a 'master' object
	JsonNode *no = json_node_alloc();
	json_node_init_object (no, o);
	json_object_set_member (jo, g_strdup_printf ("%i", id), no);

	make_text_red (status_changed);
	gtk_widget_set_sensitive (status_changed, TRUE);
	return;
}

gboolean treeview_click (GtkWidget *treeview, GdkEventButton *event, gpointer userdata) {

  /// Detect if the event was a double-click and if so, return from this function without doing anything.
	if (event->type == GDK_DOUBLE_BUTTON_PRESS) {
		return TRUE;
	}

	GtkTreePath *path;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

	/// Click on a treeview ...
	if (gtk_tree_selection_count_selected_rows(selection) <= 1) {
		/// Clear statusbar messages on any click anywhere
		gtk_statusbar_push (GTK_STATUSBAR( statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR( statusbar), ""), "");
		/// ... area that is not a file or folder
		if ( ! (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW(treeview), (gint) event->x,	(gint) event->y, &path, NULL, NULL, NULL))) {
			gtk_tree_selection_unselect_all (selection);
			return FALSE; // Let system do whatever else needs doing
		}
	}

	/// Single click with the right mouse button brings up context-menu
	if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
		/// Get path of clicked item (folder or file). If its not a folder or file, return.
		if (gtk_tree_selection_count_selected_rows(selection) <= 1) {
			if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW(treeview), (gint) event->x,	(gint) event->y, &path, NULL, NULL, NULL)) {
				/// Get tree path for row that was clicked */
				gtk_tree_selection_unselect_all (selection);
				gtk_tree_selection_select_path (selection, path);
			}  else {
				/// Click is not on a file or folder, so dont show menu */
				gtk_tree_selection_unselect_all (selection);
				return TRUE;
			}
		}

		/// Show the right click context menu
		contextmenu_show (treeview, event, path);

		return TRUE; /// We handled this
	}
	return FALSE; /// We did not handle this
}
