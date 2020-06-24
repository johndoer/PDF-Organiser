#include "loaddata.h"

void do_scan (GtkWidget *button, gpointer data);
void walk_filetree (gchar *dir);
void add_entry_to_json_object (PDFObject* e);
void add_entry_to_treemodel (GtkTreeStore * treestore, GtkTreeIter iter, PDFObject *e);


