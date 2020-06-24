#include "utils.h"
#include "scan.h"
#include "treeview.h"
#include "main.h"
#include "window_gui.h"

static const char *filters[] = { "*.pdf" };  /// Scan for files of type...
static std::vector<PDFObject*> localV;  /// need a local array variable to hold results of scan (per scan)

void do_scan (GtkWidget *button, gpointer data){

	/// Open file selector dialog to choose a folder to scan
	GtkWidget *toplevel = gtk_widget_get_toplevel (button);
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Select Directory to Scan",
	                   GTK_WINDOW(toplevel),
										 GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
	                   "OK", GTK_RESPONSE_ACCEPT,
	                   "Cancel", GTK_RESPONSE_CANCEL,
	                   NULL);

	gint res = gtk_dialog_run (GTK_DIALOG (dialog));
	if (res == GTK_RESPONSE_ACCEPT) {
    char *dirname;
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    dirname = gtk_file_chooser_get_filename (chooser);

		walk_filetree (dirname); /// Walk the directory tree

		g_free (dirname);
	} else {
		// dialog was cancelled
	}

	gtk_widget_destroy (dialog);
}

static int evaluate_scan (const char *fpath, const struct stat *info, int typeflag) {

	/// This is callback function of ftw function

	/// Process each file : if its not a pdf, continue; if its a pdf, add it to json object and treemodel
	if (typeflag == FTW_F) {
		for (guint i = 0; i < sizeof(filters) / sizeof(filters[0]); i++) {
			if (fnmatch(filters[i], fpath, FNM_CASEFOLD) == 0) {
				highestID++;
				GFile * f = g_file_new_for_path (fpath);
				gchar * fileName = g_file_get_basename (f);
				gchar* path = g_file_get_path (g_file_get_parent (f));
				int fileSize = info->st_size;

				/// Make a new PDFObject
				PDFObject * obj = g_new (PDFObject, 1);
					obj->id = highestID;
					obj->fileName = fileName;
					obj->filePath = path;
					obj->fileSize = fileSize;
					obj->parentDir = 0;
					obj->folderNo = -1;
					obj->fileNo = highestID;
					obj->position = g_strdup_printf ("%i", highestID - 1);

				/// Add PDFObject to local std::vector (from where it will be added to json-object and treemodel)
				localV.push_back(obj);

				break;
			}
		}
	}

	/// tell ftw to continue
	return 0;
}

void walk_filetree (gchar *dir){

	/// Call for recursive scan of given folder for pdf files
	ftw (dir, evaluate_scan, 16);

	/// Process found PDF files - add to jo and treemodel
	GtkTreeIter TopLevel;
	for (PDFObject * PDF : localV){
		add_entry_to_treemodel (treestore, TopLevel,  PDF);
		add_entry_to_json_object (PDF);
	}

	/// Empty vector of its contents (contents pertain to this scan only)
	localV.clear();

	/// Re-draw statusbar to clear out old and in with new
	redraw_statusbar();
	gtk_widget_set_sensitive (status_changed, TRUE);
	make_text_red(status_changed);
}

void add_entry_to_treemodel (GtkTreeStore * treestore, GtkTreeIter iter, PDFObject *e) {
	gtk_tree_store_append (treestore, &iter,  NULL);
	gtk_tree_store_set (treestore, &iter,
				0, e->id,
				1, e->fileName,
				2, e->filePath,
				3, e->fileSize,
				4, e->parentDir,
				5, e->folderNo,
				6, e->fileNo,
				7, e->position,
				-1);
}

void add_entry_to_json_object (PDFObject* e){
	JsonObject * o = json_object_new ();
		json_object_set_int_member (o, "id", e->id);
		json_object_set_string_member (o, "fileName", e->fileName);
		json_object_set_string_member (o, "filePath", e->filePath);
		json_object_set_int_member (o, "fileSize", e->fileSize);
		json_object_set_int_member (o, "parentDir", 0);
		json_object_set_int_member (o, "folderNo", -1);
		json_object_set_int_member (o, "fileNo", e->id);
		json_object_set_string_member (o, "position", e->position);

	/// Add to master object
	JsonNode *no = json_node_alloc();
	json_node_init_object (no, o);
	json_object_set_member (jo, g_strdup_printf ("%i", e->id), no);
}
