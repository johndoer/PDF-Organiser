#include "utils.h"
#include "context_menu.h"
#include "loaddata.h"
#include "treeview.h"
#include "window_gui.h"
#include "main.h"

GHashTable *table;
static const char *menuItemsArray[] = { "Open File" , "Open containing folder", "Copy filename", "Delete this item"};  // need this to get hashtable to respect order

void populate_context_menu_items(){

		/// Into the structs as cmdStr put in a pointer to a premade function

		struct MenuItems * strMenuItem_1 = g_new (MenuItems, 1);
			strMenuItem_1->label = menuItemsArray[0];
			strMenuItem_1->cmdStr = "evince open file....";
			strMenuItem_1->opt = menuaction_openfile;
		/** HASH TABLE put in */
		g_hash_table_insert(table, (gchar*)strMenuItem_1->label, (struct MenuItems*)strMenuItem_1);

		struct MenuItems * strMenuItem_2 = g_new (MenuItems, 1);
			strMenuItem_2->label =menuItemsArray[1];
			strMenuItem_2->cmdStr = "nautilus open folder....";
			strMenuItem_2->opt = menuaction_openfolder;
		/** HASH TABLE put in */
		g_hash_table_insert(table, (gchar*)strMenuItem_2->label, (struct MenuItems*)strMenuItem_2);

		struct MenuItems * strMenuItem_3 = g_new (MenuItems, 1);
			strMenuItem_3->label = menuItemsArray[2];
			strMenuItem_3->cmdStr = "copy to clipboard....";
			strMenuItem_3->opt = menuaction_copytoclipboard;
		/** HASH TABLE put in */
		g_hash_table_insert(table, (gchar*)strMenuItem_3->label, (struct MenuItems*)strMenuItem_3);

		struct MenuItems * strMenuItem_4 = g_new (MenuItems, 1);
			strMenuItem_4->label = menuItemsArray[3];
			strMenuItem_4->cmdStr = "delete item....";
			strMenuItem_4->opt = menuaction_deleteitem;
		/** HASH TABLE put in */
		g_hash_table_insert(table, (gchar*)strMenuItem_4->label, (struct MenuItems*)strMenuItem_4);

}

GtkWidget * contextmenu_make () {

	/// Make a menu from provided elements
	GtkWidget *menu = gtk_menu_new();
	GtkWidget *menuitem;

	/// Append to the right click context menu each struct item of menuItemsArray
	for (guint i = 0; i < sizeof(menuItemsArray) / sizeof(menuItemsArray[0]); i++) {
		menuitem = gtk_menu_item_new_with_label(menuItemsArray[i]);
		g_signal_connect(menuitem, "activate", (GCallback) contextmenu_item_clicked, (gpointer)menuItemsArray[i]);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}

	return menu;
}

void menuaction_openfile (gchar* id){

	/// Get the item from jo
	JsonNode * n = json_object_get_member (jo, id);
	JsonObject * o = json_node_get_object (n);

	const gchar * fileName = json_object_get_string_member (o, "fileName");                /// Get the filename
	const gchar * folderPath = json_object_get_string_member (o, "filePath");              /// Get the containing folder path
	const gchar * fullpath = g_strconcat (folderPath, G_DIR_SEPARATOR_S, fileName, NULL);  /// fullpath

	/// Check that file exists
	if ( ! g_file_test (fullpath, G_FILE_TEST_EXISTS)){
		gchar* value = g_strconcat("File '", fullpath, "' could not be found.", NULL);
		printf ("%s\n", value);
	  gtk_statusbar_push (GTK_STATUSBAR( statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR( statusbar), value), value);
	  g_free (value);
		return;
	}

	/// Open pdf file with default PDF reader
  launch_pdfreader (fullpath);
}

void menuaction_copytoclipboard (gchar* id){

	/// Copy the full file path to clipboard

	/// Get the item from jo
	JsonNode * n = json_object_get_member (jo, id);
	JsonObject * o = json_node_get_object (n);

	/// Get the filename
	const gchar * fileName = json_object_get_string_member (o, "fileName");

	/// Get the containing folder path
	const gchar * folderPath = json_object_get_string_member (o, "filePath");

	const gchar * fullpath = g_strconcat (folderPath, G_DIR_SEPARATOR_S, fileName, NULL);

	GtkClipboard *clip = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text (clip, fullpath, -1);

	return;
}

void menuaction_openfolder (gchar* id){

	/// Get the item from jo
	JsonNode * n = json_object_get_member (jo, id);
	JsonObject * o = json_node_get_object (n);

	/// Get the containing folder path
	const gchar * folderPath = json_object_get_string_member (o, "filePath");
	const gchar * fileName = json_object_get_string_member (o, "fileName");                /// Get the filename
	const gchar * fullpath = g_strconcat (folderPath, G_DIR_SEPARATOR_S, fileName, NULL);  /// fullpath

	/// Check the folder exists
	if ( ! g_file_test (folderPath, G_FILE_TEST_EXISTS)){
		gchar* value = g_strconcat("Folder '", fullpath, "' could not be found.", NULL);
		printf ("%s\n", value);
	  gtk_statusbar_push (GTK_STATUSBAR( statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR( statusbar), value), value);
	  g_free (value);
		return;
	}

	/// Launch file manager to folder path
  const gchar *command_line = g_strconcat (file_manager(), " \"", fullpath, "\"", NULL);
  printf("launching %s\n", command_line);
  GError *error = NULL;
  g_spawn_command_line_async (command_line, &error);
  if(error){
		printf("Error opening folder: %s\n", error->message);
		g_error_free(error);
		return;
	}

	gtk_window_iconify (GTK_WINDOW(window));
}

void menuaction_deleteitem (gchar* id){

	GtkTreeIter iter;
	GtkTreePath * path;
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (treeview));

	GList * list_of_selected_rows = gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION (selection), NULL);
	if( g_list_length (list_of_selected_rows)  == 1){
		path = (GtkTreePath*)list_of_selected_rows->data;
	} else {
		return; /* path describes a non-existing row - should not happen */
	}

	const gchar * filename;
	if (! gtk_tree_model_get_iter(model, &iter, path)){
		return; /* path describes a non-existing row - should not happen */
	} else {
		gtk_tree_model_get (model, &iter, 1, &filename,      -1);
	}

	/// Remove item from treemodel
	gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);

	/// Also remove from json-glib ...
	json_object_remove_member (jo, id);

	/// ... and array
	for (guint i = 0; i < PDFObjectsArray->len; i++)	{
		PDFObject * array_row = g_array_index(PDFObjectsArray, PDFObject*, i);
		if(array_row->id == atoi(id)){
			g_array_remove_index (PDFObjectsArray, i);
			break;
		}
	}

	gtk_tree_path_free (path);

	make_text_red (status_changed);
	gtk_widget_set_sensitive (status_changed, TRUE);
}

void menuaction_deletefolder (GtkWidget * x, gpointer userdata){

	GtkTreePath *path = (GtkTreePath *)userdata;
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	GtkTreeIter iter;
	gtk_tree_model_get_iter(model, &iter, path);
	int  id, parentDir;
	gchar * pos;
	gtk_tree_model_get (model, &iter,  0, &id,   4, &parentDir,  7, &pos,  -1);

	/// Set the parent
	GtkTreeIter ParentIter;
	if ( ! gtk_tree_model_iter_parent (model, &ParentIter,  &iter)){
		/// This means there is no parent, ie. its a toplevel folder
		gtk_tree_model_get_iter_from_string (model, &ParentIter, pos);
	}

	/// Set the next sibling - utterly fucking barmy.
	int ppos = 1;
	GtkTreeIter nextSiblingIter;
	gtk_tree_model_get_iter (model, &nextSiblingIter, path);
	if ( ! gtk_tree_model_iter_next (model, &nextSiblingIter)){
		/// This means there is no nextSiblingIter
		ppos = -1;
	}

	/// As it is a folder being removed, all its childrens need to be re-assigned as childrens of its parent iter
	if (gtk_tree_model_iter_has_child (model,  &iter)){
		for (gint i = 0; i < gtk_tree_model_iter_n_children (model, &iter); i++){

			/// Get a ChildIter pointing to the child item in question
			GtkTreeIter ChildIter;
			gtk_tree_model_iter_nth_child (model, &ChildIter, &iter, i);

			/// Copy the entire contents of the ChildIter into variables. So wtf good are these f'ing iters good for then ffs!.
			int childID, fileSize, parentDir, folderNo, fileNo;
			gchar* fileName, *filePath, *position;
			gtk_tree_model_get (model, &ChildIter,
					0, &childID,
					1, &fileName,
					2, &filePath,
					3, &fileSize,
					4, &parentDir,
					5, &folderNo,
					6, &fileNo,
					7, &position,
					-1);

			/// Re-parent ChildIter to be a child of ParentIter
			/// The row will be empty after this function is called. To fill in values, you need to call gtk_tree_store_set()
			if (ppos != -1){
				gtk_tree_store_insert_before (treestore, &ChildIter, NULL, &nextSiblingIter);
			} else {
				gtk_tree_store_insert (treestore, &ChildIter, &ParentIter, -1);
			}

			/// (Re)Set the values into the iter
			gtk_tree_store_set (treestore, &ChildIter,
				0, childID,    //obj->id,
				1, fileName,   //obj->fileName,
				2, filePath,   //,
				3, fileSize,   // initial fileSize - count of children
				4, parentDir,  // initial where its been placed (parentDir)
				5, folderNo,   // no folder no - its a file
				6, fileNo,     // fileNo - highest id
				7, position,   // initially 0 - will hold position, text path eg 0:3:3
				-1);

			/// Re-set 'parentDir' and 'position' position fields of ChildID item in jo items in jo
			/// (We're skipping doing the same in GArray for reason of not wanting to iterate over whole array)
			JsonNode * n = json_object_get_member (jo,  g_strdup_printf ("%i", childID));
			JsonObject * o = json_node_get_object (n);
			json_object_set_string_member (o, "position", position);  /// <<<<<<<<< there is a ne position
			json_object_set_int_member (o, "parentDir", parentDir);

			g_free(position);
			g_free(fileName);
			g_free(filePath);
		}
	}

	/// Remove folder from treemodel
	gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);

	gtk_tree_path_free(path);
	return;
}

void contextmenu_item_clicked (GtkWidget * mm, gpointer item){

	/** STRUCT METHOD  */
	gchar * key = (gchar*)item;

	/** HASH TABLE get_out */
	struct MenuItems * strct =  (struct MenuItems *)g_hash_table_lookup(table,  key);

	/** C++ std::map */  /** makes binary jump from 46k to 86k* /
	auto search = map.find(key);
	if (search != map.end()) {
			std::cout << "Found " << search->first << " " << search->second << '\n';
	} else {
			std::cout << "Not found\n";
	}
	*/
	/** summary:
	 * C++ is too bloatware
	 * glib hashtable is succinct, but i think only one key-value pair
	 * struct is the way to go - put in as many items as you want
	 * - the issue, for all of them, is where to do the populating - would like to have done it in function on the fly
	 * looks like you are going to have to construct an array of structs - another one. just for menu items
	 * menuentry -> its onclick action, and whatever else...
	 * oh the joy of javascripts arrays/objects
	 * json-glib objects ???
	*/
	// try:
	// 1. can you put a struct in a glib hashtable  -- yes, it seems you can
	// 2. can you put a function in a struct -- so yes, you _can_ put a function in a struct, _as a pointer to function_ - somewhat convoluted
	//                                                  but then hey, this is c after all. wouldnt count if it wasnt convoluted.
	// 2. cont. so, it seems we dont need to go the elaborate route of c function pointers
	//              instead we can use the struct as if it was a c++ class and put the function in it.
	//gchar* z = value->opt((gchar*)"x1", (gchar*)"x22");


	/// We need to get which item was selected
	GtkTreePath *path;
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	GtkTreeIter iter;
	int  id;
	GList * list_of_selected_rows = gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION (selection), NULL);
	if( g_list_length (list_of_selected_rows)  == 1){
		path = (GtkTreePath*)list_of_selected_rows->data;
	} else {
		printf("multiple selections not supported\n");
		return;
	}

	if (! gtk_tree_model_get_iter(model, &iter, path)){
		return; /* path describes a non-existing row - should not happen */
	}
	gtk_tree_path_free(path);
	gtk_tree_model_get (model, &iter,  0, &id,    -1);

	/// Perform the action, passing to it the id of file
	strct->opt((gchar*)g_strdup_printf("%i", id));

}

void view_context_menu_onDoSomething (GtkWidget *menuitem, gpointer userdata) {

	/// DELETE AN ITEM

	GtkTreeIter iter;
	GtkTreePath * path;
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (treeview));

	GList * list_of_selected_rows = gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION (selection), NULL);
	if( g_list_length (list_of_selected_rows)  == 1){
		path = (GtkTreePath*)list_of_selected_rows->data;
	} else {
		return; // no path
	}
	gtk_tree_model_get_iter (model, &iter,  path);

	if (! gtk_tree_model_get_iter(model, &iter, path))
		return; /* path describes a non-existing row - should not happen */


	/// Get json object member name (id)
	int  id;
	gtk_tree_model_get (model, &iter,  0, &id,       -1);

	/// Remove item from treemodel
	gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);

	/// Also remove from json-glib ...
	json_object_remove_member (jo, g_strdup_printf ("%i", id));

	/// ... and array
	for (guint i = 0; i < PDFObjectsArray->len; i++)	{
		PDFObject * array_row = g_array_index(PDFObjectsArray, PDFObject*, i);
		if(array_row->id == id){
			g_array_remove_index (PDFObjectsArray, i);
			break;
		}
	}

	gtk_tree_path_free (path);

	make_text_red (status_changed);
	gtk_widget_set_sensitive (status_changed, TRUE);
}

void contextmenu_show (GtkWidget *treeview, GdkEventButton *event, gpointer userdata) {

	/// Shows the context menu on right click

	GtkTreePath *path = (GtkTreePath *)userdata;
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
	GtkTreeIter iter;
	gtk_tree_model_get_iter(model, &iter, path);
	int  id, fileNo;
	gtk_tree_model_get (model, &iter,  0, &id,   6, &fileNo,    -1);

	/// Show menu for folder click
	if(fileNo < 0){
		GtkWidget *menu, *menuitem;
		menu = gtk_menu_new();
		menuitem = gtk_menu_item_new_with_label("Delete this folder");
		g_signal_connect(menuitem, "activate", (GCallback) menuaction_deletefolder, path);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

		gtk_widget_show_all(menu);
		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
		(event != NULL) ? event->button : 0,	gdk_event_get_time((GdkEvent*)event));

		return;
	}

	/// Show regular menu for file items
	GtkWidget * menu = contextmenu_make ();

	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
	(event != NULL) ? event->button : 0,	gdk_event_get_time((GdkEvent*)event));
}
