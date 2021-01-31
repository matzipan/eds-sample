/*
 * Copyright (C) 2019  Andrei-Costin Zisu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ENVOYER_MAILCOREINTERFACE_IMAP_H

#define ENVOYER_MAILCOREINTERFACE_IMAP_H

#include <gee.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif
void* mail_core_interface_imap_connect (gchar* username, gchar* access_token);
void* mail_core_interface_imap_update_access_token (void* session, gchar* access_token);
void mail_core_interface_imap_fetch_folders (void* session, GAsyncReadyCallback callback, void* user_data);
GeeLinkedList* mail_core_interface_imap_fetch_folders_finish (GTask *task);
void mail_core_interface_imap_fetch_messages (void* session, gchar* folder_path, guint64 start_uid_value, guint64 end_uid_value, gboolean flags_only, GAsyncReadyCallback callback, void* user_data);
GeeLinkedList* mail_core_interface_imap_fetch_messages_finish (GTask *task);
void mail_core_interface_imap_store_flags_for_messages (void* voidSession, gchar* folder_path, GeeList* message_uids, GAsyncReadyCallback callback, void* user_data);
gboolean mail_core_interface_imap_store_flags_for_messages_finish (GTask *task);
void mail_core_interface_imap_move_messages (void* voidSession, gchar* source_folder_path, GeeList* message_uids, gchar* destination_folder_path, GAsyncReadyCallback callback, void* user_data);
gboolean mail_core_interface_imap_move_messages_finish (GTask *task);
void mail_core_interface_imap_get_html_for_message (void* session, gchar* folder_path, void* envoyer_message, GAsyncReadyCallback callback, void* user_data);
const gchar* mail_core_interface_imap_get_html_for_message_finish (GTask *task);
void mail_core_interface_imap_get_plain_text_for_message (void* session, gchar* folder_path, void* envoyer_message, GAsyncReadyCallback callback, void* user_data);
const gchar* mail_core_interface_imap_get_plain_text_for_message_finish (GTask *task);
void mail_core_interface_imap_idle_listener (void* session, gchar* folder_path, guint64 last_known_id, GAsyncReadyCallback callback, void* user_data);
gboolean mail_core_interface_imap_idle_listener_finish (GTask *task);
void mail_core_interface_imap_fetch_data_for_message_part (void* voidSession, gchar* folder_path, guint64 uid, gchar* part_id, gint64 encoding, GAsyncReadyCallback callback, void* user_data);
GBytes* mail_core_interface_imap_fetch_data_for_message_part_finish (GTask *task);
void mail_core_interface_imap_get_message_uids_for_folder (void* session, gchar* folder_path, GAsyncReadyCallback callback, void* user_data);
GeeLinkedList* mail_core_interface_imap_get_message_uids_for_folder_finish (GTask *task);


#ifdef __cplusplus
}
#endif

#endif
