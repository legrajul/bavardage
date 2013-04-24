using Gtk;

namespace Bavardage.Widgets {

    public class AboutDialog: Gtk.AboutDialog {
        public AboutDialog (Gtk.Window parent) {
            this.set_destroy_with_parent (true);
            this.set_transient_for (parent);
            this.set_modal (true);

            this.artists = {"Charles Ango", "Julien Legras"};
            this.authors = {"Charles Ango", "Ismaël Kabore", "Julien Legras", "Yves Nouafo", "Jean-Baptiste Souchal"};

            this.license = "Logiciel développé dans le cadre d'un projet universitaire encadré par Magali Bardet";
            this.wrap_license = true;

            this.program_name = "Bavardage";
            this.comments = "Messagerie instantanée";
            this.copyright = "Copyright © 2012-2013";
            this.version = "0.1";

            this.website = "http://github.com/legrajul/bavardage";
            this.website_label = "Dépôt github";
        }
    }
}