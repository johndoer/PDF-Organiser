
typedef void (*Operation)(gchar*); // For function pointer

struct MenuItems {
  const gchar  *label;
  const gchar  *cmdStr;
  Operation opt;  // funtion pointer
};

extern GHashTable *table;
void populate_context_menu_items();
GtkWidget * contextmenu_make ();
void menuaction_openfile (gchar* id);
void menuaction_copytoclipboard (gchar* id);
void menuaction_openfolder (gchar* id);
void menuaction_deleteitem (gchar* id);
void menuaction_deletefolder (GtkWidget * x, gpointer userdata);
void contextmenu_item_clicked (GtkWidget * mm, gpointer item);
void view_context_menu_onDoSomething (GtkWidget *menuitem, gpointer userdata);
void contextmenu_show (GtkWidget *treeview, GdkEventButton *event, gpointer userdata);
