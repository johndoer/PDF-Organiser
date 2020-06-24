#include "utils.h"
#include "loaddata.h"
#include "main.h"

GArray * PDFObjectsArray = g_array_new (FALSE, FALSE, sizeof ( PDFObject* ));
GArray * folders = g_array_new (FALSE, FALSE, sizeof ( iterItem* ));
GtkTreeStore * treestore;
GtkWidget * treeview;
JsonObject * jo;


void prepare_treemodel () {
	treestore = gtk_tree_store_new(
			8,                   // col count
			G_TYPE_INT,          // 0 id
			G_TYPE_STRING,       // 1 fileName
			G_TYPE_STRING,       // 2 filePath
			G_TYPE_INT,          // 3 fileSize 2 - size Kb, Mb, Gb etc
			G_TYPE_INT,          // 4 parent dir
			G_TYPE_INT,          // 5 folderNo
			G_TYPE_INT,          // 6 fileNo
			G_TYPE_STRING        // 7 position
			);
  treeview = gtk_tree_view_new_with_model( GTK_TREE_MODEL(treestore));
  g_object_unref(treestore);
}

void load_data () {

	/// Init treemodel
	prepare_treemodel ();

	/// Init json obect
	if (g_file_test (datafilepath(), G_FILE_TEST_EXISTS) ) {
		JsonParser * parser = json_parser_new ();
		GError * error = NULL;
	  json_parser_load_from_file (parser, datafilepath(), &error);
		if (error != NULL){
			fprintf (stderr, "Error: Problem reading json file:\n %s\n", error->message);
			g_error_free (error);
			return;
		}

		JsonNode * n = json_parser_get_root (parser);
		g_assert (JSON_NODE_HOLDS_OBJECT (n));

		jo = json_node_get_object (n);

		load_data_from_file (); /// Populate json-object
	} else {
		jo = json_object_new ();
	}
}

void load_data_from_file (){

	/// For each object member, add it to PDFObjectsArray on initial load
	json_object_foreach_member (jo, iterate_json_object, NULL);

	/// Sort PDFObjectsArray ascending on path(position)
	g_array_sort (PDFObjectsArray, (GCompareFunc)sort_array);

	/// Each folder needs to have its own iterator, (for its childrens to be appended to)
	/// and based on fileNo(?) (which will be its childrens' parentDir)
	/// Traverse PDFObjectsArray adding folders and files to treemodel at their right positions.
	GtkTreeIter ChildLevel;
	for (guint i = 0; i < PDFObjectsArray->len; i++)	{

		PDFObject * e = g_array_index(PDFObjectsArray, PDFObject*, i);

		/// Process folders
		if ( e->folderNo > 0 ){

			/// If this folder is highest number, set it as so
			if (e->folderNo > highestFolder ) highestFolder = e->folderNo;

			/// If the entry in question is a folder that is child of another folder...
			if(e->parentDir > 0){
				GtkTreeIter parentIter;
				for (guint j = 0; j < folders->len; j++)	{
					iterItem * comparee =  g_array_index (folders, iterItem*, j);
					if (comparee->fileNo == e->parentDir){
						parentIter = comparee->iter;
						break;
					}
				}
				gtk_tree_store_append (treestore, &ChildLevel, &parentIter);
				gtk_tree_store_set(treestore, &ChildLevel,
						0, e->id,
						1, e->fileName,
						2, e->filePath,
						3, e->fileSize,
						4, e->parentDir,
						5, e->folderNo,
						6, e->fileNo,
						7, e->position,
						-1);

				iterItem * newIterItem = g_new (iterItem, 1);
				newIterItem->fileNo = e->id;
				newIterItem->iter = ChildLevel;
				g_array_append_val (folders, newIterItem);
			} else {
				GtkTreeIter newIter;
				gtk_tree_store_append (treestore, &newIter,  NULL);
				gtk_tree_store_set (treestore, &newIter,
						0, e->id,
						1, e->fileName,
						2, e->filePath,
						3, e->fileSize,
						4, e->parentDir,
						5, e->folderNo,
						6, e->fileNo,
						7, e->position,
						-1);

				/// Add to folders GArray a mapping of fileNo to iterator, for later iter retrieval based on fileNo
				iterItem * newIterItem = g_new (iterItem, 1);
				newIterItem->fileNo = e->id;
				newIterItem->iter = newIter;
				g_array_append_val (folders, newIterItem);
			}
		}

		/// Process Files.
		if ( (e->folderNo < 0) && (e->fileNo > 0) ){
			// retrieve iter to append this child to, based on parentDir
			GtkTreeIter parentIter;
			if(e->parentDir == 0){
				gtk_tree_store_append(treestore, &ChildLevel, NULL); /// Top level item
			} else {
				for (uint j = 0; j < folders->len; j++)	{
					iterItem * comparee =  g_array_index(folders, iterItem*, j);
					if(comparee->fileNo == e->parentDir){
						parentIter = comparee->iter; /// Find the folder to set as parent
						break;
					}
				}
				gtk_tree_store_append(treestore, &ChildLevel, &parentIter);
			}
			gtk_tree_store_set(treestore, &ChildLevel,
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
	}
}

void iterate_json_object (JsonObject *object, const gchar *member_name, JsonNode *member_node,  gpointer user_data) {

	/// For each object member, add it to PDFObjectsArray on initial load
	/// (PDFObjectsArray as a GArray is needed in order to sort the items by their positions)
	JsonObject * j = json_node_get_object (member_node);

	PDFObject * e = g_new (PDFObject, 1);
			JsonNode * n = json_object_get_member (j, "id");
			e->id = (int)json_node_get_int(n);
			if(e->id > highestID)  highestID = e->id;
			n = json_object_get_member (j, "fileName");
			e->fileName = (char*)json_node_get_string(n);
			n = json_object_get_member (j, "filePath");
			e->filePath = (char*)json_node_get_string(n);
			n = json_object_get_member (j, "fileSize");
			e->fileSize = (int)json_node_get_int(n);
			n = json_object_get_member (j, "parentDir");
			e->parentDir = (int)json_node_get_int(n);
			n = json_object_get_member (j, "folderNo");
			e->folderNo = (int)json_node_get_int(n);
			n = json_object_get_member (j, "fileNo");
			e->fileNo = (int)json_node_get_int(n);
			n = json_object_get_member (j, "position");
			e->position = (char*)json_node_get_string(n);
	g_array_append_val (PDFObjectsArray, e);

	/// Add entries to the treemodel too (after you sort the array)
	/// ... is done in caller
}

int sort_array (const void *a, const void *b){

	/// This function receives *strings* such as "3:4:1", "2:6:0" to compare to each other and sort *numerically*.

	const PDFObject *entry1 = *(PDFObject **)a;
	const PDFObject *entry2 = *(PDFObject **)b;

	gchar ** asplit = g_strsplit (entry1->position, ":", -1);
	gchar ** bsplit = g_strsplit (entry2->position, ":", -1);
	uint i = 0; int alen = 1; while ( i <= strlen (entry1->position)) { (entry1->position[i] == ':' ) && alen++; i++; }
	uint j = 0; int blen = 1; while ( j <= strlen (entry2->position)) { (entry2->position[j] == ':' ) && blen++; j++; }

	int n = 0;
	while ( n <= alen && n <=blen ){
		int aa = (alen > n) ? atoi (asplit[n]) : -1;
		int bb = (blen > n) ? atoi (bsplit[n]) : -1;
		if ( aa > bb ){ return 1; } else if ( aa < bb ){ return -1; } else { n++; }
	}

	return 0;

	/// With above algo you have unlimited depth recursion.
}

