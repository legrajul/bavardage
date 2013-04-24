using Gtk;
using Bavardage.ClientCore;
using Bavardage.Common;

namespace Bavardage.Widgets {
    public class NewRoomDialog: Gtk.Dialog {

        private Entry room_name_entry;

        private CheckButton secure_room_checkbox;

        public signal void create_new_room (string room_name, bool is_secure);

        public NewRoomDialog (Gtk.Window parent, Gtk.Application app) {
            this.add_buttons (Gtk.Stock.CANCEL, Gtk.ResponseType.CANCEL, Gtk.Stock.NEW, Gtk.ResponseType.ACCEPT);
            this.set_title ("Créer un salon");
            this.set_modal (true);
            this.set_transient_for (parent);
            this.set_position (WindowPosition.CENTER_ON_PARENT);
            this.set_application (app);
            setup_view ();
            this.show_all ();
            this.check_resize ();
            connect_signals ();
        }

        private void setup_view () {
            var box = this.get_content_area ();
            box.set_orientation (Orientation.VERTICAL); {
                room_name_entry = new Entry ();
                secure_room_checkbox = new CheckButton.with_label ("salon sécurisé");
                box.pack_start (room_name_entry, true, true);
                box.pack_start (secure_room_checkbox, true, true);

            }
        }

        private void connect_signals () {
            room_name_entry.activate.connect ( () => { this.response (Gtk.ResponseType.ACCEPT);});
            this.response.connect ((response_id) => {
                    if (response_id == Gtk.ResponseType.ACCEPT) {
                        this.create_new_room (room_name_entry.get_text (), secure_room_checkbox.get_active ());
                    }
                    this.hide_on_delete ();
                });
        }
    }
}