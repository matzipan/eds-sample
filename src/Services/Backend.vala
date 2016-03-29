public class Notes.Services.Backend {
    private E.SourceRegistry registry;
    private Notes.Services.Session session;
    
    public async Backend() {
        Camel.init(E.get_user_data_dir(), false);
        
        session = new Notes.Services.Session(Path.build_filename (E.get_user_data_dir(), "mail"), Path.build_filename (E.get_user_data_dir(), "mail"));
        
        registry = yield new E.SourceRegistry (null); 
        
        get_mail_account_sources().foreach((source_item) => {
                var extension = source_item.get_extension(E.SOURCE_EXTENSION_MAIL_ACCOUNT);           
                // setup autorefresh?  https://git.gnome.org/browse/evolution/tree/libemail-engine/e-mail-session.c#n495
 

                var service = session.add_service(source_item.get_uid(), ((E.SourceBackend) extension).get_backend_name(), Camel.ProviderType.STORE);
                
                E.SourceCamel.configure_service(source_item, service); //@TODO
                
                message("%s", session.online ? "Online" : "Not online");
                
                message("%s", ((E.SourceMailAccount) extension).get_needs_initial_setup() ? "Needs setup" : "Does not need setup");
                
                ((Camel.OfflineStore) service).set_online_sync(true);

                ((Camel.OfflineStore) service).connect_sync();

                GLib.HashTable<weak string,weak string> out_save_setup;
                 
                ((Camel.OfflineStore) service).initial_setup_sync(out out_save_setup); // https://developer.gnome.org/camel/3.19/CamelStore.html#camel-store-initial-setup-sync
                
                ((Camel.Store) service).synchronize_sync(true);
                
            });
    }
    
    public void set_online() {
        session.set_online(true);
    }
    
    public void set_offline() {
        session.set_online(false);
    }
    
    public GLib.List<E.Source> get_mail_account_sources() {
        var sources = registry.list_sources(E.SOURCE_EXTENSION_MAIL_ACCOUNT);
        
        sources.foreach((source_item) => {
                if(source_item.get_uid() == "local" || 
                    source_item.get_uid() == "vfolder") {
                        sources.remove_all(source_item);
                    }
            });

        return sources.copy();
    }
    
    public GLib.List<Camel.Service> get_services() {
        return session.list_services().copy();
    }
    
    public GLib.List<E.Source> get_mail_transport_sources() {
        return null; //@TODO
    }
}