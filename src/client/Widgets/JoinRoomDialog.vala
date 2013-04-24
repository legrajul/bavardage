using Gtk;
using Bavardage.ClientCore;
using Bavardage.Common;

namespace Bavardage.Widgets {
    public class JoinRoomDialog: Gtk.Dialog {

        public Entry room_name_entry;

        public JoinRoomDialog (Gtk.Window parent, Gtk.Application app) {
            this.add_buttons (Gtk.Stock.CANCEL, Gtk.ResponseType.CANCEL, Gtk.Stock.OK, Gtk.ResponseType.ACCEPT);
            this.set_title ("Rejoindre un salon");
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
                box.pack_start (room_name_entry, true, true);
            }
        }

        private void connect_signals () {
            room_name_entry.activate.connect ( () => { this.response (Gtk.ResponseType.ACCEPT);});
        }

    }
}