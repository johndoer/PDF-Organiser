#include "utils.h"
#include "window_gui.h"
#include "main.h"
#include "scan.h"

GtkWidget *window;
GtkWidget *sw;
GtkWidget * statusbar;
GtkWidget * scan_button;
GtkWidget * status_changed;


void draw_main_window (){

	/// Window
	window = gtk_application_window_new (GTK_APPLICATION (app));
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_title(GTK_WINDOW(window), "PDF Organiser");
	int scrWidth  = gdk_screen_get_width (gdk_screen_get_default ());
	int scrHeight = gdk_screen_get_height (gdk_screen_get_default ());
	int window_width  = scrWidth * .65;
	int window_height = scrHeight * .70;
  gtk_widget_set_size_request(window, window_width, window_height);
	gtk_window_set_icon_name (GTK_WINDOW (window), "pdforganiser");
	///-------------------------------

	/// VBox
  GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_container_add(GTK_CONTAINER(window), vbox);
	///-------------------------------


	/// Scrolled Window
	sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_ETCHED_IN);
  gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 5);
	///-------------------------------
	/// Statusbar
  statusbar = gtk_statusbar_new ();
  gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, TRUE, 1);
  draw_statusbar ();

  gtk_widget_show_all(window);
	gtk_application_add_window (app, GTK_WINDOW(window));
}

void redraw_statusbar (){
	if ( PDFObjectsArray->len > 0 ){
		gtk_statusbar_push (GTK_STATUSBAR( statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR( statusbar), ""), "");
	}
	gtk_widget_show_all(window);
}

void draw_statusbar (){

	gtk_statusbar_push (GTK_STATUSBAR( statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR( statusbar), ""), "");

	/// Save State button
	status_changed = gtk_button_new_with_label ("Save Current State");
	gtk_container_add(GTK_CONTAINER(statusbar), status_changed);
	gtk_button_set_relief (GTK_BUTTON(status_changed), GTK_RELIEF_NONE);
	gtk_widget_set_sensitive (status_changed, FALSE);

	/// Add File button
	GtkWidget * add_file_btn = gtk_button_new_with_label ("Add File");
	gtk_container_add (GTK_CONTAINER (statusbar), add_file_btn);

	/// New Folder button
	GtkWidget * new_folder_btn = gtk_button_new_with_label ("Add Folder");
	gtk_container_add(GTK_CONTAINER(statusbar), new_folder_btn);

	/// Scan button
	scan_button = gtk_button_new_with_label ("Scan");
	gtk_container_add(GTK_CONTAINER( statusbar), scan_button);
	gtk_widget_set_tooltip_text (scan_button, "Scan for PDF files");

	g_signal_connect (add_file_btn, "clicked", G_CALLBACK(add_file), window);
	g_signal_connect (new_folder_btn, "clicked", G_CALLBACK(add_folder), treeview);
	g_signal_connect (status_changed, "clicked", G_CALLBACK(save_changes), treeview);
	g_signal_connect ( scan_button, "clicked", G_CALLBACK(do_scan), NULL);

	/// Message to scan system for PDF files if nothing in datafile.
	if ( PDFObjectsArray->len < 1) {
		gchar* value = g_strconcat("PDF files list is empty. Click 'Scan' button to scan for PDF files.", NULL);
	  gtk_statusbar_push (GTK_STATUSBAR( statusbar),
	        gtk_statusbar_get_context_id(GTK_STATUSBAR( statusbar), value), value);
	}
}

void add_folder (GtkWidget * widget, gpointer user_data) {

	/// When you add a New Folder, add it to the folder you are currently in.
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (treeview));
	GList * list_of_selected_rows = gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION (selection), NULL);
	GtkTreeIter new_iter;
	GtkTreePath * path;
	if( g_list_length (list_of_selected_rows)  == 0){
		/// There is no current selection - so add to end of treeview
		gtk_tree_store_insert (treestore,  &new_iter,   NULL,   -1); /// <<< adds to end of current level
		path = gtk_tree_model_get_path (GTK_TREE_MODEL(treestore), &new_iter);
	} else if( g_list_length (list_of_selected_rows)  == 1){
		/// There is a current selection - is it a folder or a file
		path = (GtkTreePath*)list_of_selected_rows->data;
		GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
		gtk_tree_model_get_iter(model, &new_iter, path);

		int folderNo;
		gtk_tree_model_get (model, &new_iter,   5,&folderNo,     -1);
		if (folderNo == -1){
			/// Selection is a file so add to its parent (by moving path up)
			gtk_tree_path_up (path);
		}
		GtkTreeIter parentiter;
		gtk_tree_model_get_iter (model,  &parentiter,  path);
		gtk_tree_store_insert (treestore,  &new_iter,   &parentiter,   0); /// <<< add to beginning of level
	}	else {
		/// Mutli row selections not as yet supported
		return;
	}

	/// Add onto end of array
	highestID++;
	highestFolder++;

	PDFObject * obj = g_new (PDFObject, 1);
		obj->id = highestID;
		obj->fileName = (gchar*)"New Folder";
		obj->filePath = NULL;
		obj->fileSize = 0;
		obj->parentDir = 0;
		obj->folderNo = highestFolder;
		obj->fileNo = -1;
		obj->position = gtk_tree_path_to_string (path);
	g_array_append_val (PDFObjectsArray, obj);

	/// Append to model
	gtk_tree_store_set (treestore, &new_iter,
		0, obj->id,         // highest id no
		1, obj->fileName,   // Currently 'New Folder', user can change it
		2, NULL,            // No path for folders
		3, obj->fileSize,   // initial fileSize - count of children
		4, obj->parentDir,  // initial where its been placed (parentDir)
		5, obj->folderNo,   // highest folder no
		6, -1,              // fileNo its not a file
		7, obj->position,   // initially 0 - will hold position, text path eg 0:3:3
		-1);

	/// Add a new object_member to jo json object
	JsonObject * o = json_object_new ();
		json_object_set_int_member (o, "id", obj->id);
		json_object_set_string_member (o, "fileName", obj->fileName);
		json_object_set_null_member(o, "filePath");
		json_object_set_int_member (o, "fileSize", 0);
		json_object_set_int_member (o, "parentDir", 0);
		json_object_set_int_member (o, "folderNo", obj->folderNo);
		json_object_set_int_member (o, "fileNo", -1);
		json_object_set_string_member (o, "position", obj->position);
	/// Add to master object
	JsonNode *no = json_node_alloc();
	json_node_init_object (no, o);
	json_object_set_member (jo, g_strdup_printf ("%i", obj->id), no);

	make_text_red (status_changed);
	gtk_widget_set_sensitive (status_changed, TRUE);
	return;
}

void add_file (GtkWidget *button, GtkWidget *parent_window){

	/// Open file selector dialog to manually choose a pdf to add
	GtkWidget *toplevel = gtk_widget_get_toplevel (button);
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open File",
	      GTK_WINDOW(toplevel),
	      GTK_FILE_CHOOSER_ACTION_OPEN,
	      "OK", GTK_RESPONSE_ACCEPT,
	      "Cancel", GTK_RESPONSE_CANCEL,
	      NULL);

	GtkFileFilter *filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*.pdf");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog),filter);

	gint res = gtk_dialog_run (GTK_DIALOG (dialog));
	if (res == GTK_RESPONSE_ACCEPT) {
    char * filename;
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    filename = gtk_file_chooser_get_filename (chooser);

    if( ! g_str_has_suffix (filename,  ".pdf")) {
			// should not ever get to here as we have a filter in place
			printf("not pdf\n");
			g_free (filename);
			gtk_widget_destroy (dialog);
			return;
		}

		/// When you add a new File, add it to the folder you are currently in.
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (treeview));
		GList * list_of_selected_rows = gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION (selection), NULL);
		GtkTreeIter new_iter;
		GtkTreePath * path;
		int folderNo;
		if( g_list_length (list_of_selected_rows)  == 0){
			/// There is no current selection - so add to end of treeview
			gtk_tree_store_insert (treestore,  &new_iter,   NULL,   -1); /// <<< adds to end
			path = gtk_tree_model_get_path (GTK_TREE_MODEL(treestore), &new_iter);
			folderNo = 0;
		} else if ( g_list_length (list_of_selected_rows)  == 1){
			/// There is a current selection - is it a folder or a file
			path = (GtkTreePath*)list_of_selected_rows->data;
			GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
			gtk_tree_model_get_iter(model, &new_iter, path);

			gtk_tree_model_get (model, &new_iter,   5,&folderNo,     -1);
			if (folderNo == -1){
				/// Selection is a file so add to its parent (by moving path up)
				gtk_tree_path_up (path);
			}
			GtkTreeIter parentiter;
			gtk_tree_model_get_iter (model,  &parentiter,  path);
			gtk_tree_store_insert (treestore,  &new_iter,   &parentiter,   0); /// <<< add to beginning of level
		}	else {
			/// Mutli row selections not as yet supported
			return;
		}

		/// Add onto end of array
		highestID++;
		highestFolder++;

		GFile * f = g_file_new_for_path (filename);
		gchar * ff = g_file_get_basename (f);
		gchar* ppath = g_file_get_path (g_file_get_parent (f));

		GError **error = NULL;
		GFileInfo * file_info = g_file_query_info (f,
					       G_FILE_ATTRIBUTE_STANDARD_SIZE ","
					       G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
					       G_FILE_QUERY_INFO_NONE, NULL, error);

		/// Reserved for possible future use
		//goffset bytes = g_file_info_get_size (file_info);
		//gchar *mime_type = g_strdup (g_file_info_get_content_type (file_info));

		PDFObject * obj = g_new (PDFObject, 1);
			obj->id = highestID;
			obj->fileName = ff;
			obj->filePath = ppath;
			obj->fileSize = g_file_info_get_size (file_info);
			obj->parentDir = folderNo;
			obj->folderNo = -1;  // Its a file - folder no is -1
			obj->fileNo =  highestID;
			obj->position = gtk_tree_path_to_string (path); /// Position where it was added
		g_array_append_val (PDFObjectsArray, obj);

		/// Append to model
		gtk_tree_store_set (treestore, &new_iter,
			0, obj->id,         // highest id no
			1, obj->fileName,
			2, obj->filePath,
			3, obj->fileSize,   // initial fileSize - count of children
			4, obj->parentDir,  // initial where its been placed (parentDir)
			5, obj->folderNo,   // no folder no - its a file
			6, obj->fileNo,     // fileNo - highest id
			7, obj->position,   // initially 0 - will hold position, text path eg 0:3:3
			-1);

		/// Add a new object_member to jo json object
		JsonObject * o = json_object_new ();
			json_object_set_int_member (o, "id", obj->id);
			json_object_set_string_member (o, "fileName", obj->fileName);
			json_object_set_string_member(o, "filePath", obj->filePath);
			json_object_set_int_member (o, "fileSize", obj->fileSize);
			json_object_set_int_member (o, "parentDir", obj->parentDir);
			json_object_set_int_member (o, "folderNo", obj->folderNo);
			json_object_set_int_member (o, "fileNo", obj->fileNo);
			json_object_set_string_member (o, "position", obj->position);
		/// Add to master object
		JsonNode *no = json_node_alloc();
		json_node_init_object (no, o);
		json_object_set_member (jo, g_strdup_printf ("%i", obj->id), no);

		g_free (filename);
		g_object_unref (file_info);
	} else {
		// dialog was cancelled
	}

	gtk_widget_destroy (dialog);


	make_text_red (status_changed);
	gtk_widget_set_sensitive (status_changed, TRUE);
	return;
}

void print_json_object () {

	/// The root json object
	JsonNode * n = json_node_alloc();
	json_node_init_object (n, jo);
	JsonGenerator * g = json_generator_new ();
	json_generator_set_pretty (g, TRUE);
	json_generator_set_root (g, n);

	/// generate a json string
	gchar *d = json_generator_to_data (g, NULL);

	/// print to stdout or write to a file
	gboolean stdout = FALSE;

	if (stdout) {
		g_print("%s\n", d);
		g_free(d);
		json_node_free(n);
		return;
	}

	GError * error = NULL;
	g_file_set_contents (datafilepath(), d,  -1, &error);
	if(error){
		printf("Error writing to file: %s", error->message);
		g_error_free(error);
	}

	/// Cleanup
	g_free(d);
	json_node_free(n);
}

gboolean iterate_treemodel (GtkTreeModel *tree_model, GtkTreePath *path,  GtkTreeIter *iter,  gpointer data) {

	/** Instead of writing coordinates after every move,
	// do it just before writesave, and for ALL of them
	// find all their positions - and write it to their place, then save...
	// idea is that things will be where you left them on next load
	*/

	/// get the id
	int id;
	gtk_tree_model_get (tree_model, iter, 0, &id,      -1);

  ///write the path to position in jo object
	JsonNode * n = json_object_get_member (jo,  g_strdup_printf ("%i", id));
	JsonObject * o = json_node_get_object (n);
	json_object_set_string_member (o, "position", gtk_tree_path_to_string (path));

	/// add the added-to object to a 'master' object
	JsonNode *no = json_node_alloc();
	json_node_init_object (no, o);
	json_object_set_member (jo, g_strdup_printf ("%i", id), no);

	return FALSE;
}

void save_changes (GtkWidget * widget, gpointer user_data) {

	/// Traverse treemodel to place position values
	gtk_tree_model_foreach (GTK_TREE_MODEL(treestore),  iterate_treemodel,   NULL);

	/// Write to file
	print_json_object ();

	/// Reset button text color back to its greyish color
	GtkWidget * label = gtk_bin_get_child (GTK_BIN(status_changed));;
	GdkRGBA rgba;
	gdk_rgba_parse (&rgba,  "rgba(255, 255, 255, .5)");
	gtk_widget_override_color (label, GTK_STATE_FLAG_NORMAL, &rgba);

	gtk_widget_set_sensitive (status_changed, FALSE);
}

void make_text_red (GtkWidget * status_changed) {
	GtkWidget * label = gtk_bin_get_child (GTK_BIN(status_changed));;
	GdkRGBA rgba;
	gdk_rgba_parse (&rgba,  "rgba(255, 0, 0, 1)");
	gtk_widget_override_color (label, GTK_STATE_FLAG_NORMAL, &rgba);
}
