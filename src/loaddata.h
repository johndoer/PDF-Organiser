
struct PDFObject{
	gint id;
	gchar * fileName;
	gchar * filePath;
	gint fileSize;
	gint parentDir;
	gint folderNo;
	gint fileNo;
	gchar * position;
};

struct iterItem {
	int fileNo;
	int id;
	GtkTreeIter iter;
};

extern GArray * PDFObjectsArray;
extern GArray * folders;
extern GtkTreeStore * treestore;
extern GtkWidget * treeview;
extern JsonObject * jo;

void load_data ();
void load_data_from_file ();
void iterate_json_object (JsonObject *object, const gchar *member_name, JsonNode *member_node,  gpointer user_data);
int sort_array (const void *a, const void *b);
void prepare_treemodel ();
