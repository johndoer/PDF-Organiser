iconsdir = $(datadir)/icons/hicolor

nobase_dist_icons_DATA = 	  16x16/apps/pdforganiser.svg     22x22/apps/pdforganiser.svg     24x24/apps/pdforganiser.svg     32x32/apps/pdforganiser.svg     48x48/apps/pdforganiser.svg     scalable/apps/pdforganiser.svg

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(iconsdir)

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then echo "Updating Gtk icon cache."; $(gtk_update_icon_cache); else echo "*** Icon cache not updated.  After (un)install, run this:"; echo "***   $(gtk_update_icon_cache)"; 	fi
