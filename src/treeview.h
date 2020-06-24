
void make_treeview ();
void set_tree_cell_text (GtkTreeViewColumn *tc, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
void set_tree_cell_pixbuf (GtkTreeViewColumn *tc, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
void filesize_Kib (GtkTreeViewColumn *tc, GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
void on_dblclick_row (GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata);
void cell_edited_callback (GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer user_data);
void on_drag_data_inserted (GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data);
void on_row_changed (GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data);
gboolean treeview_click (GtkWidget *treeview, GdkEventButton *event, gpointer userdata);
