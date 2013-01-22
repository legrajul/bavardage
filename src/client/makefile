VALAC=valac
PKGS= --pkg gtk+-3.0 --pkg gmodule-2.0

all: Client

Client: Client.vala interface.ui
	$(VALAC) $(PKGS) Client.vala

clean:
	rm Client
