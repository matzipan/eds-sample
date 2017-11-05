/*
 * Copyright 2016 Andrei-Costin Zisu
 *
 * This software is licensed under the GNU Lesser General Public License
 * (version 2.1 or later).  See the COPYING file in this distribution.
 */

#include <MailCore/MCIMAPAsyncSession.h>
#include <MailCore/MCOperationCallback.h>
#include <MailCore/MCIMAPOperationCallback.h>
#include <MailCore/MCIMAPFetchMessagesOperation.h>
#include <MailCore/MCMessageHeader.h>
#include <MailCore/MCIMAPMessage.h>
#include <MailCore/MCAddress.h>
#include <glib.h>
#include <gee.h>
#include "envoyer.h"

EnvoyerModelsAddress* get_as_envoyer_address (mailcore::Address* address) {
    return envoyer_models_address_new (
            address->displayName () == NULL ? "" : address->displayName ()->UTF8Characters (),
            address->mailbox ()->UTF8Characters ()
        );
}

GeeLinkedList* get_as_list_of_envoyer_addresses (mailcore::Array* addresses) {
    GeeLinkedList* list = gee_linked_list_new (ENVOYER_MODELS_TYPE_ADDRESS, (GBoxedCopyFunc) g_object_ref, g_object_unref, NULL, NULL, NULL);

    if(addresses != NULL) {
        for(uint i = 0 ; i < addresses->count () ; i++) {
            mailcore::Address* address = (mailcore::Address*) addresses->objectAtIndex (i);

            gee_abstract_collection_add ((GeeAbstractCollection*) list, get_as_envoyer_address (address));
        }
    }

    return list;
}

class MailCoreInterfaceIMAPFetchMessagesByUIDCallback : public mailcore::OperationCallback, public mailcore::IMAPOperationCallback {
public:
    MailCoreInterfaceIMAPFetchMessagesByUIDCallback (GTask* task) {
            this->task = task;
    }

    virtual void operationFinished(mailcore::Operation * op) {
        //@TODO check IMAPOperation::error

        auto messages = ((mailcore::IMAPFetchMessagesOperation *) op)->messages();
        auto vanished_messages = ((mailcore::IMAPFetchMessagesOperation *) op)->vanishedMessages(); //@TODO

        auto list = gee_linked_list_new (ENVOYER_MODELS_TYPE_MESSAGE, (GBoxedCopyFunc) g_object_ref, g_object_unref, NULL, NULL, NULL);

        for(uint i = 0 ; i < messages->count () ; i++) {
            auto message = (mailcore::IMAPMessage*) messages->objectAtIndex (i);

            auto from_address = get_as_envoyer_address (message->header ()->from ());
            auto sender_address = get_as_envoyer_address (message->header ()->sender ());

            auto to_addresses = get_as_list_of_envoyer_addresses (message->header ()->to ());
            auto cc_addresses = get_as_list_of_envoyer_addresses (message->header ()->cc ());
            auto bcc_addresses = get_as_list_of_envoyer_addresses (message->header ()->bcc ());

            auto references_list = gee_linked_list_new (G_TYPE_STRING, (GBoxedCopyFunc) g_strdup, g_free, NULL, NULL, NULL);

            auto references = message-> header()->references ();

            if (references != NULL) {
                for(uint j = 0 ; j < references->count (); j++) {
                    mailcore::String* reference = (mailcore::String*) references->objectAtIndex (j);

                    gee_abstract_collection_add ((GeeAbstractCollection*) references_list, reference->UTF8Characters ());
                }
            }

            message->retain(); //@TODO this should be called from Envoyer.Models.Message constructor

            EnvoyerModelsMessage* message_model = envoyer_models_message_new (
                message,
                from_address,
                sender_address,
                (GeeCollection*) to_addresses,
                (GeeCollection*) cc_addresses,
                (GeeCollection*) bcc_addresses,
                message->header ()->subject ()->UTF8Characters (),
                message->header ()->receivedDate (),
                (GeeCollection*) references_list,
                message->header ()->messageID ()->UTF8Characters (),
                message->uid (),
                message->sequenceNumber (),
                (message->flags () & mailcore::MessageFlagSeen) != 0,
                (message->flags () & mailcore::MessageFlagFlagged) != 0,
                (message->flags () & mailcore::MessageFlagDeleted) != 0,
                (message->flags () & mailcore::MessageFlagDraft) != 0
            );

            gee_abstract_collection_add ((GeeAbstractCollection*) list, message_model);
        }

        messages->release();
        //@TODO also release when Envoyer.Models.Message is deleted.

        g_task_return_pointer (task, list, g_object_unref);

        g_object_unref (task);
        delete this;
    }

private:
    GTask* task;
};

extern "C" void mail_core_interface_imap_fetch_messages (mailcore::IMAPAsyncSession* session,
                                                        gchar* folder_path,
                                                        guint starting_uid_value,
                                                        guint end_uid_value,
                                                        gboolean flags_only,
                                                        GAsyncReadyCallback callback,
                                                        void* user_data) {
    auto task = g_task_new (NULL, NULL, callback, user_data);

    //@TODO this won't work with on 32 bit, becasue of UINT64
    auto uidRange = mailcore::IndexSet::indexSetWithRange (mailcore::RangeMake (starting_uid_value, UINT64_MAX));

    int kind;
    if (flags_only) {
        kind = mailcore::IMAPMessagesRequestKindFlags;
    } else {
        kind = mailcore::IMAPMessagesRequestKindHeaders |
            mailcore::IMAPMessagesRequestKindFlags |
            mailcore::IMAPMessagesRequestKindStructure |
            mailcore::IMAPMessagesRequestKindGmailLabels |
            mailcore::IMAPMessagesRequestKindGmailThreadID |
            mailcore::IMAPMessagesRequestKindGmailMessageID;
    }

    auto fetch_messages_operation = session->fetchMessagesByUIDOperation(new mailcore::String (folder_path), (mailcore::IMAPMessagesRequestKind) kind, uidRange);

    auto session_callback = new MailCoreInterfaceIMAPFetchMessagesByUIDCallback(task);

    // fetch_messages_operation->setImapCallback(session_callback); @TODO for progress feedback
    ((mailcore::Operation *) fetch_messages_operation)->setCallback (session_callback);

    fetch_messages_operation->start();
}

extern "C" GeeLinkedList* mail_core_interface_imap_fetch_messages_finish (GTask *task) {
    g_return_val_if_fail (g_task_is_valid (task, NULL), NULL);

    return (GeeLinkedList*) g_task_propagate_pointer (task, NULL);
}
