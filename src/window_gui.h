
extern GtkWidget * window;
extern GtkWidget * sw;
extern GtkWidget * statusbar;
extern GtkWidget * scan_button;
extern GtkWidget * status_changed;

void draw_main_window ();
void draw_statusbar ();
void redraw_statusbar ();
void add_folder (GtkWidget * widget, gpointer user_data);
void add_file (GtkWidget *button, GtkWidget *parent_window);
void print_json_object ();
gboolean iterate_treemodel (GtkTreeModel *tree_model, GtkTreePath *path,  GtkTreeIter *iter,  gpointer data);
void save_changes (GtkWidget * widget, gpointer user_data);
void make_text_red (GtkWidget * status_changed);


