using Gtk;

namespace Bavardage.Widgets {
    public class SendInvitationDialog: Gtk.Dialog {
        public Entry entry_login { get; set; }

        public SendInvitationDialog (Gtk.Window parent, Gtk.Application app) {
            this.add_buttons (Gtk.Stock.CANCEL, Gtk.ResponseType.CANCEL, Gtk.Stock.ADD, Gtk.ResponseType.ACCEPT);
            this.set_title ("Envoyer une invitation");
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
                var hbox = new Box (Orientation.HORIZONTAL, 0); {
                    entry_login = new Entry ();
                    hbox.pack_start (new Label ("Pseudo de l'invité : "));
                    hbox.pack_start (entry_login, true, true);
                }
                box.pack_start (hbox, true, true);
            }
        }

        private void connect_signals () {
            entry_login.activate.connect ( () => { this.response (Gtk.ResponseType.ACCEPT);});
        }
    }
}