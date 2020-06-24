#include "utils.h"
#include "main.h"
#include "loaddata.h"
#include "window_gui.h"
#include "treeview.h"



GtkApplication *app;
int highestID;
int highestFolder;

const gchar *pdf_reader(){
	/// Default PDF reader on user's system
	const gchar *filetype = "application/pdf";
	GAppInfo *app_info = g_app_info_get_default_for_type (filetype, FALSE);
	return g_app_info_get_executable (app_info);
}

const gchar *file_manager(){
	/// Default File Manager on user's system
	const gchar *filetype = "inode/directory";
	GAppInfo *app_info = g_app_info_get_default_for_type (filetype, FALSE);
	return g_app_info_get_executable (app_info);
}

void launch_pdfreader(const gchar *fullpath){
  const gchar *command_line = g_strconcat (pdf_reader(), " \"", fullpath, "\"", NULL);
  GError *error = NULL;
  g_spawn_command_line_async (command_line, &error);
  if(error){
		printf("Error opening PDF: %s\n", error->message);
		g_error_free(error);
		return;
	}
}

const gchar *config_dir(){
	return g_strconcat (g_get_user_config_dir (), G_DIR_SEPARATOR_S, "pdforganiser", NULL);
}

const gchar *datafilepath(){
	return g_strconcat (config_dir(), G_DIR_SEPARATOR_S, "datafile.json", NULL);
}

void set_config(){
	/// Look for user configuration directory; If not exists, make one
	if ( ! g_file_test (config_dir (), G_FILE_TEST_IS_DIR)){
		g_mkdir_with_parents (config_dir(), 0644);
	}
}

static void startup (GtkApplication *app, gpointer  user_data) {
	highestID = 0;
	highestFolder = 0;

	set_config();
	load_data();
	draw_main_window();
	make_treeview();
}

static void activate (GtkApplication *app, gpointer  user_data) {
	GtkWindow * window = gtk_application_get_active_window (app);
	gtk_widget_show_all(GTK_WIDGET(window));
	gtk_window_present (window);
	gtk_window_activate_focus (window);
}

int main (int argc, char **argv) {
  app = gtk_application_new ("org.carefree.pdforganiser", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  g_signal_connect (app, "startup", G_CALLBACK (startup), NULL);
  int status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
