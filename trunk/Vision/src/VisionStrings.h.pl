/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Vision.
 *
 * The Initial Developer of the Original Code is The Vision Team.
 * Portions created by The Vision Team are
 * Copyright (C) 1999, 2000, 2001 The Vision Team.  All Rights
 * Reserved.
 *
 * Contributor(s): Wade Majors <wade@ezri.org>
 *                 Rene Gollent
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Jamie Wilkinson
 */

/* 
Spolszczenie stworzone przez BeTeamPL http://wwww.BeTeamPL.prv.pl
Wersja Spolszczenia: 0.9

Spolszczone przez Piotra Maksymiuka "M0V1" 17.4.2003
Zaczęte 16.4.2003 21:15
Skończone 17.4.2003 0:18 (hehe, w jedną noc! :))
pierwsza wersja spolszczenia, niektóre komendy są "dziwnie" przetłumaczone.
Poprawki nastąpią po skompilowaniu tego, gdy będę mógł zobaczyć gdzie popełniłem błędy.
wszelkie błedy zgłaszać na "Movi@o2.pl"
----------
17.04.2003
Małe poprawki w tłumaczeniu - Billcow (billcow@irc.pl)
----------
18.04.2003
Kolejne poprawki oraz test - Mixer (mixer@beos.pl)
----------


*/

#ifndef _VISIONSTRINGS_H_
#define _VISIONSTRINGS_H_

// channel agent

#define S_CHANNEL_INIT              "*** Rozmawiasz teraz na "
#define S_CHANNEL_REJOIN            "*** Próba ponownego połšczenia się z "
#define S_CHANNEL_RECON_REJOIN      "[@] Próba ponownego połšczenia"
#define S_CHANNEL_GOT_KICKED        "*** Zostałeś wyrzucony z "
#define S_CHANNEL_GOT_KICKED2       "przez"
#define S_CHANNEL_SET_MODE          " ustaw tryb "

// channel options

#define S_CHANOPTS_TITLE            " Opcje"
#define S_CHANOPTS_OPID1            "Jesteś operatorem kanału "
#define S_CHANOPTS_OPID2            "Możesz zmienić każdą z tych opcji"

// client agent

#define S_CLIENT_DCC_SUCCESS        "Zakończone "
#define S_CLIENT_DCC_FAILED         "Anulowane "
#define S_CLIENT_DCC_SENDTYPE       "wysłane "
#define S_CLIENT_DCC_RECVTYPE       "odebrane "
#define S_CLIENT_DCC_TO             " do "
#define S_CLIENT_DCC_FROM           " z "
#define S_CLIENT_DCC_SIZE_UNITS     " bajtów"
#define S_CLIENT_DCC_SPEED_UNITS    " cps"

// client window

#define S_CW_APP_ABOUT              "O Prgramie"
#define S_CW_APP_PREFS              "Ustawienia"
#define S_CW_APP_CHANLIST           "Lista Kanałów"
#define S_CW_APP_TERMINAL           "Nowy Terminal"
#define S_CW_APP_QUIT               "Wyjdz"
#define S_CW_SERVER_MENU            "Serwer"
#define S_CW_SERVER_CONNECT         "Połącz z"
#define S_CW_SERVER_SETUP           "Konfiguracja"
#define S_CW_EDIT_MENU              "Edytuj"
#define S_CW_EDIT_CUT               "Wytnij"
#define S_CW_EDIT_COPY              "Kopiuj"
#define S_CW_EDIT_PASTE             "Wklej"
#define S_CW_EDIT_SELECT_ALL        "Zaznacz wszystko"
#define S_CW_WINDOW_MENU            "Okno"
#define S_CW_WINDOW_PART            "Zamknij podokno"
#define S_CW_WINDOW_UP              "Góra"
#define S_CW_WINDOW_DOWN            "Dół"
#define S_CW_WINDOW_SM_UP           "Sprytna Góra"
#define S_CW_WINDOW_SM_DOWN         "Sprytny Dół"
#define S_CW_WINDOW_NETWORK         "Okno Sieci"
#define S_CW_WINDOW_PREVIOUS        "Poprzednie Okno"
#define S_CW_WINDOW_NET_UP          "Przesuń Sieć w górę"
#define S_CW_WINDOW_NET_DOWN        "Przesuń Sieć w dół"
#define S_CW_WINDOW_COLLAPSE        "Zwiń Sieć"
#define S_CW_WINDOW_EXPAND          "Rozwiń Sieć"

// client window dock

#define S_CWD_WINLIST_HEADER        "Lista Okien"
#define S_CWD_NOTIFY_HEADER         "Lista Powiadomień"

// DCC messages

#define S_DCC_SPEED                 "bps: "
#define S_DCC_ESTABLISH_ERROR       "Nie udało się nawiązać poąłczenia."
#define S_DCC_ESTABLISH_SUCCEEDED   "Połączenie nawiązane."
#define S_DCC_CONNECT_TO_SENDER     "Łączenie z nadawcą."
#define S_DCC_RECV1                 "Odbieranie \""
#define S_DCC_RECV2                 "\" od "
#define S_DCC_SEND1                 "Wysyłanie \""
#define S_DCC_SEND2                 "\" do "
#define S_DCC_LOCK_ACQUIRE          "Odbieranie klucza DCC"
#define S_DCC_ACK_WAIT              "Czekanie na pozwolenie"
#define S_DCC_LISTEN_CALL           "Wykonywanie rozmowy słuchającej."
#define S_DCC_WAIT_FOR_CONNECTION   "Oczekiwanie na połączenie "
#define S_DCC_WRITE_ERROR           "Błąd w zapisywaniu danych."
#define S_DCC_SOCKET_ERROR          "Błąd w tworzeniu gniazdka.\n"
#define S_DCC_BIND_ERROR            "Błąd w przypinaniu gniazda.\n"
#define S_DCC_CONN_ERROR            "Błąd w łšczeniu gniazda.\n"
#define S_DCC_CHAT_LISTEN           "Przyjmowanie połączenia z adresem "
#define S_DCC_CHAT_TRY              "Próba połączenia z adresem "
#define S_DCC_CHAT_PORT             ", port "
#define S_DCC_CHAT_CONNECTED        "Połączony!\n"
#define S_DCC_CHAT_TERM             "Rozmowa DCC zakończona.\n"

// list agent

#define S_LIST_MENU                 "Kanały"
#define S_LIST_MENU_FIND            "Znajdż"
#define S_LIST_MENU_FINDNEXT        "Znajdż następny"
#define S_LIST_MENU_FILTER          "Filtruj"
#define S_LIST_COLUMN_CHAN          "Kanał"
#define S_LIST_COLUMN_USER          "Użytkownicy"
#define S_LIST_COLUMN_TOPIC         "Temat"
#define S_LIST_STATUS_LOADING       "Ładowanie"
#define S_LIST_STATUS_DONE          "Zrobione"
#define S_LIST_PROMPT_TITLE         "Znajdż"
#define S_LIST_PROMPT_LABEL         "    Znajdż:"

// network prefs -- server list

#define S_PREFSERVER_STATUS_COLUMN  "Stan"
#define S_PREFSERVER_SERVER_COLUMN  "Serwer"
#define S_PREFSERVER_PORT_COLUMN    "Port"
#define S_PREFSERVER_ADD_BUTTON     "Dodaj"
#define S_PREFSERVER_REMOVE_BUTTON  "Wyrzuć"
#define S_PREFSERVER_EDIT_BUTTON    "Edytuj"
#define S_PREFSERVER_DESC1          "Klucz: "
#define S_PREFSERVER_DESC2          "  * = Pierwszorzędny"
#define S_PREFSERVER_DESC3          "  + = Opcjonalny (Gdy pierwszy nie działa)"
#define S_PREFSERVER_DESC4          "  - = Wyłączony"
#define S_PREFSERVER_OK_BUTTON      "OK"
#define S_PREFSERVER_SEL_STRING     "Wybierz serwery dla "

// network prefs -- main view

#define S_NETPREFS_NETMENU          "Sieci"
#define S_NETPREFS_DEFAULTS_ITEM    "Domyślnie"
#define S_NETPREFS_ADD_NEW          "Dodaj Nowy"
#define S_NETPREFS_REMOVE           "Usuń teraźniejszy"
#define S_NETPREFS_DUPE             "Duplikuj teraźniejszy"
#define S_NETPREFS_NET_BOX          "Detale sieci"
#define S_NETPREFS_PERSONAL_BOX     "Detale osobowe"
#define S_NETPREFS_CONN1            "Połączony z"
#define S_NETPREFS_CONN2            "Powrót do 9 innych."
#define S_NETPREFS_CHANGE_SERVER    "Zmień serwery"
#define S_NETPREFS_AUTOEXEC         "Autoexec:"
#define S_NETPREFS_LAG_CHECK        "Włącz sprawdzanie opóżnienia"
#define S_NETPREFS_STARTUP_CONN     "Połącz się z tym serwerem przy starcie Visiona"
#define S_NETPREFS_USE_DEFAULTS     "Użyj domyślnych ustawień"
#define S_NETPREFS_PREFNICK         "Preferowane Nicki:"
#define S_NETPREFS_ADD_BUTTON       "Dodaj"
#define S_NETPREFS_REMOVE_BUTTON    "Usuń"
#define S_NETPREFS_IDENT            "Identyfikacja: "
#define S_NETPREFS_REALNAME         "Prawdziwe imie: "
#define S_NETPREFS_FALLBACK1        "Powrót do "
#define S_NETPREFS_FALLBACK2        " inne"
#define S_NETPREFS_FALLBACK2_PLURAL "i"
#define S_NETPREFS_ADDNET_TITLE     "Dodaj sieci"
#define S_NETPREFS_DUPENET_TITLE    "Duplikuj sieci"
#define S_NETPREFS_NET_PROMPT       "Nazwa sieci: "
#define S_NETPREFS_ADDNICK_TITLE    "Dodaj nicka"
#define S_NETPREFS_ADDNICK_PROMPT   "Nick: "

// network windows

#define S_NETWORK_WINDOW_TITLE      "Ustawienia Sieci"
#define S_SERVERPREFS_TITLE         "Serwery"

// notify list

#define S_NOTIFYLIST_QUERY_ITEM     "Prywatnie"
#define S_NOTIFYLIST_WHOIS_ITEM     "Kto to jest?"
#define S_NOTIFYLIST_REMOVE_ITEM    "Usuń"
#define S_NOTIFYLIST_DCC_ITEM       "Rozmowa DCC"
// command parser

#define S_PCMD_PARAMETER_ERROR      "Błąd: Niewłaściwe parametry"
#define S_PCMD_SEND_TITLE           "Wysyłanie pliku do "
#define S_PCMD_SEND_BUTTON          "Wyślij"
#define S_PCMD_TRY_NEW_NICK         "*** Próba nowego nicka "
#define S_PCMD_SET_BOOL_SUCCESS     "Bool został ustawiony"
#define S_PCMD_SET_BOOL_FAILURE     "Błąd ustawiania boola"
#define S_PCMD_DNS1                 "Ustalone "
#define S_PCMD_DNS2                 " do "
#define S_PCMD_DNS_ERROR            "Nie można ustalić "
#define S_PCMD_PEXEC_ERROR          "/pexec: komenda się nie powiodła "
#define S_PCMD_VIS_UPTIME           "Vision jest uruchomiony od "
// ctcp parser

#define S_PCTCP_RESPONSE            " odpowiedż"
#define S_PCTCP_SECOND              "sekunda"
#define S_PCTCP_SECOND_PLURAL       "sekundy"

// numerics parser

#define S_PENUM_CURMODE             "[x] twuj tryb to: "
#define S_PENUM_WHOWAS              "[był]"
#define S_PENUM_IDLE                "Bezczynny: "
#define S_PENUM_SIGNON              "Połączony od: "
#define S_PENUM_WHOIS_CHANNELS      "[x] Kanały: "
#define S_PENUM_CHANMODE            "*** Tryb kanału dla "
#define S_PENUM_MLOCK               "*** Channel lock for " //What the hell is this??
#define S_PENUM_CHANCREATED         "stworzony"
#define S_PENUM_NO_TOPIC            "[x] Nie ma ustawionego tematu dla "
#define S_PENUM_DISP_TOPIC          "*** Temat: "
#define S_PENUM_TOPIC_SET_BY        "*** Temat ustalony przez "
#define S_PENUM_INVITING            " został zaproszony do "
#define S_PENUM_NAMEREPLY           "*** Użytkowników na "
#define S_PENUM_SERVER_MOTD         "- Wiadomość Dnia:"
#define S_PENUM_RECON_SUCCESS       "[@] Pomyślne ponowne połączenie"
#define S_PENUM_NICKINUSE1          "* Nick \""
#define S_PENUM_NICKINUSE2          "\" w użyciu lub niedostępne.. próbowanie \""
#define S_PENUM_NICKINUSE3          "[x] Nick/Kanał "
#define S_PENUM_NICKINUSE4          " już w użyciu lub niedostępne."
#define S_PENUM_ALLNICKSUSED1       "* Wszystkie twoje zdefiniowane Nicki są już w użyciu."
#define S_PENUM_ALLNICKSUSED2       "* Napisz /NICK <Nowy_Nick> any spróbować innych nicków."
#define S_PENUM_NOTINCHANNEL        " nie ma na "
#define S_PENUM_ME_NOTINCHANNEL     "[x] Nie jesteś na "
#define S_PENUM_ALREADYINCHANNEL    " już jest na "
#define S_PENUM_KEY_ALREADY_SET     "[x] Klucz kanału jest już ustawiony na "
#define S_PENUM_UNKNOWNCHANMODE     "[x] Nieznany tryb kanału: '"
#define S_PENUM_INVITE_ONLY         " (Tylko zaproszeni)"
#define S_PENUM_BANNED              " (Zakaz wstępu na kanał)"
#define S_PENUM_BADCHANKEY          " (niepoprawny klucz kanału)"
#define S_PENUM_UNKNOWNMODE         "[x] Nieznana flaga MODE."

// events parser

#define S_PEVENTS_UMODE_CHANGE      "*** Tryb użytkownika zmieniony: "
#define S_PEVENTS_INVITE1           "*** Zostałeś zaproszony do "
#define S_PEVENTS_INVITE2           " przez "
#define S_PEVENTS_SILENCE_ADDED     "*** Hostmask dodał do listy CISZA: "
#define S_PEVENTS_SILENCE_REMOVED   "*** Hostmask usunał z listy CISZA: "

// app prefs

#define S_PREFAPP_VERSION_PARANOID  "Pokazuj informacje o OSie w odpowiedzi na komende VERSION"
#define S_PREFAPP_CMDW              "Wymagaj podwujnego Cmd+Q/W aby zamknąć"
#define S_PREFAPP_STRIP_MIRC        "Wyłącz kolory mIRCa"
#define S_PREFAPP_WARN_MULTILINE    "Warn when multiline pasting"
#define S_PREFAPP_QUERY_MSG         "Ostrzegaj przed wklejaniem kilku linijek tekstu"

// color prefs

#define S_PREFCOLOR_TEXT            "Tekst"
#define S_PREFCOLOR_BACKGROUND      "Tło"
#define S_PREFCOLOR_URL             "URL"
#define S_PREFCOLOR_SERVERTEXT      "Tekst Serwera"
#define S_PREFCOLOR_NOTICE          "Powiadom"
#define S_PREFCOLOR_ACTION          "Akcja"
#define S_PREFCOLOR_QUIT            "Wyjście"
#define S_PREFCOLOR_ERROR           "Błąd"
#define S_PREFCOLOR_NICK_EDGES      "Brzegi Nicka"
#define S_PREFCOLOR_UNICK_EDGES     "Brzegi nicka użytkownika"
#define S_PREFCOLOR_NICK_TEXT       "Tekst nicka"
#define S_PREFCOLOR_JOIN            "Wejdż"
#define S_PREFCOLOR_KICK            "Wykop"
#define S_PREFCOLOR_WHOIS           "Kto to jest?"
#define S_PREFCOLOR_NAMES_NORM      "Nicki (Normalne)"
#define S_PREFCOLOR_NAMES_OP        "Nicki (Operatorów Kanałów[+o])"
#define S_PREFCOLOR_NAMES_HELP      "Nicki (Pomocników)"
#define S_PREFCOLOR_NAMES_VOICE     "Nicki (z wyróżnieniem [+v])"
#define S_PREFCOLOR_NAMES_SEL       "Wybór Nicków"
#define S_PREFCOLOR_NAMES_BG        "Tła Nickó"
#define S_PREFCOLOR_CTCP_REQ        "Prośba CTCP"
#define S_PREFCOLOR_CTCP_RPY        "Odpowiedż CTCP"
#define S_PREFCOLOR_IGNORE          "Ignoruj"
#define S_PREFCOLOR_INPUT_TXT       "Wpisywanie tekstu"
#define S_PREFCOLOR_INPUT_BG        "Tło wpisywania"
#define S_PREFCOLOR_WLIST_NORM      "Normalny Status Winlista"
#define S_PREFCOLOR_WLIST_TXT       "Status tekstu Winlista"
#define S_PREFCOLOR_WLIST_NICK      "Status powiadomienia o nicku Winlista"
#define S_PREFCOLOR_WLIST_SEL       "Status selekcji Winlista"
#define S_PREFCOLOR_WLIST_EVT       "Status wydarzeń Winlista"
#define S_PREFCOLOR_WLIST_BG        "Tło Winlista"
#define S_PREFCOLOR_WALLOPS         "Wallops" // Co to qrwa jest??
#define S_PREFCOLOR_TIMESTAMP       "Czas"
#define S_PREFCOLOR_TIMESTAMP_BG    "Tło czasu"
#define S_PREFCOLOR_SELECTION       "Zakreślanie tekstu"
#define S_PREFCOLOR_MIRCWHITE       "mIRC Biały"
#define S_PREFCOLOR_MIRCBLACK       "mIRC Czarny"
#define S_PREFCOLOR_MIRCDBLUE       "mIRC Ciemny Niebieski"
#define S_PREFCOLOR_MIRCGREEN       "mIRC Zielony"
#define S_PREFCOLOR_MIRCRED         "mIRC Czerwony"
#define S_PREFCOLOR_MIRCBROWN       "mIRC Bršzowy"
#define S_PREFCOLOR_MIRCPURPLE      "mIRC Fioletowy"
#define S_PREFCOLOR_MIRCORANGE      "mIRC Pomaranzowy"      
#define S_PREFCOLOR_MIRCYELLOW      "mIRC Żółty"
#define S_PREFCOLOR_MIRCLIME        "mIRC Limonowy"
#define S_PREFCOLOR_MIRCTEAL        "mIRC Turkusowy"
#define S_PREFCOLOR_MIRCAQUA        "mIRC Jasny niebieski"
#define S_PREFCOLOR_MIRCLBLUE       "mIRC Niebieski "
#define S_PREFCOLOR_MIRCPINK        "mIRC Różowy"
#define S_PREFCOLOR_MIRCGREY        "mIRC Szary"
#define S_PREFCOLOR_MIRCSILVER      "mIRC Srebrny"
#define S_PREFCOLOR_NOTIFY_ON       "Powiadamiaj Online"
#define S_PREFCOLOR_NOTIFY_OFF      "Powiadamiaj Offline"
#define S_PREFCOLOR_NOTIFY_BG       "Kolor listy Powiadomień"
#define S_PREFCOLOR_NOTIFY_SEL      "Selekcja listy Powiadomień"
#define S_PREFCOLOR_REVERT          "Zmień Spowrotem"

// command prefs

#define S_PREFCOMMAND_QUIT          "Wyjście:"
#define S_PREFCOMMAND_KICK          "Wykop:"
#define S_PREFCOMMAND_IGNORE        "Ignoruj:"
#define S_PREFCOMMAND_UNIGNORE      "Nie ignoruj:"
#define S_PREFCOMMAND_AWAY          "Z dala od klawiatury:"
#define S_PREFCOMMAND_BACK          "Wróć:"
#define S_PREFCOMMAND_UPTIME        "Czas działania:" //jak to po PL dobrze zrobic?? Billcow: wlasnie tak :)

// dcc prefs

#define S_PREFDCC_BLOCK_SIZE        "Rozmiar bloków DCC: "
#define S_PREFDCC_AUTOACK           "Automatycznie akceptuj przychodzące pliki"
#define S_PREFDCC_PRIVATE           "Automatycznie sprawdzaj obecność NAT IP"
#define S_PREFDCC_DEFPATH           "Domyślna ścieżka: "
#define S_PREFDCC_PORTRANGE         "Zakres portów DCC"
#define S_PREFDCC_PORTMIN           "Min: "
#define S_PREFDCC_PORTMAX           "Max: "

// event prefs

#define S_PREFEVENT_JOIN            "Wejdż:"
#define S_PREFEVENT_PART            "Odejdż:"
#define S_PREFEVENT_NICK            "Nick:"
#define S_PREFEVENT_QUIT            "Wyjdż:"
#define S_PREFEVENT_KICK            "Wykop:"
#define S_PREFEVENT_TOPIC           "Temat:"
#define S_PREFEVENT_SNOTICE         "Uwagi Serwera:"
#define S_PREFEVENT_UNOTICE         "Uwagi użytkownika:"
#define S_PREFEVENT_NOTIFYON        "Włšcz powiadomienia:"
#define S_PREFEVENT_NOTIFYOFF       "Wyłšcz powiadomienia:"

// font prefs

#define S_PREFFONT_TEXT             "Tekst"
#define S_PREFFONT_SMESSAGES        "Wiadomości serwera"
#define S_PREFFONT_URLS             "URLe"
#define S_PREFFONT_NAMESLIST        "Lista imion"
#define S_PREFFONT_INPUT_TEXT       "Tekst"
#define S_PREFFONT_WINLIST          "Lista Okien"
#define S_PREFFONT_CHANLIST         "Lista Kanałów"
#define S_PREFFONT_TSTAMP           "Znacznik czasu"
#define S_PREFFONT_FONTLABEL        "Czcionka: "
#define S_PREFFONT_SIZELABEL        "Rozmiar: "

// log prefs

#define S_PREFLOG_LOGPATH           "Bazowa ścieżka logów:"
#define S_PREFLOG_TS_FORMAT         "Format znacznika czasu:"
#define S_PREFLOG_SHOW_TIMESTAMP    "Pokazuj znaczniki czasu w oknie IRC"
#define S_PREFLOG_USE_LOGGING       "Włšcz logowanie"
#define S_PREFLOG_LOG_TIMESTAMP     "Dodaj znacznik czasu do nazw plików logów"
#define S_PREFLOG_ALERT_TITLE       "Błąd"
#define S_PREFLOG_ALERT_TEXT        "ścieżka logów, którą wprowadziłeś jest błędna."
#define S_PREFLOG_ALERT_BUTTON      "OK"

// main prefs view

#define S_PREFGEN_APP_ITEM          "Applikacja"
#define S_PREFGEN_COLOR_ITEM        "Kolory"
#define S_PREFGEN_FONT_ITEM         "Czcionki"
#define S_PREFGEN_COMMAND_ITEM      "Komendy"
#define S_PREFGEN_EVENT_ITEM        "Wydarzenia"
#define S_PREFGEN_DCC_ITEM          "DCC"
#define S_PREFGEN_LOG_ITEM          "Logowanie"

// preferences window

#define S_PREFSWIN_TITLE            "Ustawienia"

// server agent

#define S_SERVER_ATTEMPT1           "[@] Próba "
#define S_SERVER_ATTEMPT2           "ponowne"
#define S_SERVER_ATTEMPT3           "łączenie (próba "
#define S_SERVER_ATTEMPT4           " z "
#define S_SERVER_ATTEMPT5           "[@] Próba połączenia z "
#define S_SERVER_CONN_ERROR1        "[@] Nie można było się połączyć z adresem i portem. Sprawdż czy towje połączenie z internetem działa."
#define S_SERVER_CONN_ERROR2        "[@] Nie można było połączyć sie z serwerem. Przepraszamy."
#define S_SERVER_CONN_OPEN          "[@] Połączenie nawiązane, czekanie na odpowiedż"
#define S_SERVER_LOCALIP_ERROR      "[@] Błąd przy zdobywaniu lokalnego IP"
#define S_SERVER_LOCALIP            "[@] Lokalne IP: "
#define S_SERVER_PROXY_MSG          "[@] (Wygląda na to, że jesteś za Bramką internetową. Vision wyśle adres twojej bramki do serwera IRC po pomyślnym połączeniu z serwerem. Będzie to używane do komunikacji DCC.)"
#define S_SERVER_PASS_MSG           "[@] Wysyłanie hasła"
#define S_SERVER_HANDSHAKE          "[@] Negocjowanie"
#define S_SERVER_ESTABLISH          "[@] Zakończone"
#define S_SERVER_RETRY_LIMIT        "[@] Limit prób osiągnięty; kończenie. Wpisz /reconnect Jeśli chcesz spróbować jeszcze raz."
#define S_SERVER_DISCONNECT         "[@] Zostałeś rozłączony z "
#define S_SERVER_DISCON_STATUS      "Nie połączony"
#define S_SERVER_CONN_PROBLEM       "PROBLEM Z POŁĄCZENIEM"
#define S_SERVER_LAG_DISABLED       "Niekatywny"
#define S_SERVER_DCC_CHAT_PROMPT    " chce z tobą rozpocząć rozmowe przez DCC."
#define S_SERVER_WAITING_RETRY      "[@] Oczekiwanie "
#define S_SERVER_WAITING_SECONDS    " sekund"
#define S_SERVER_WAITING_PLURAL     ""
#define S_SERVER_WAITING_ENDING     " przed kolejną próbą"

// server entry window

#define S_SERVERWIN_TITLE           "Dodaj serwer"
#define S_SERVERWIN_SERVER          "Serwer: "
#define S_SERVERWIN_PORT            "Port: "
#define S_SERVERWIN_MENU1           "Wybierz status"
#define S_SERVERWIN_MENU_PRI        "Pierwszorzędny"
#define S_SERVERWIN_MENU_SEC        "Opcjonalny"
#define S_SERVERWIN_MENU_DIS        "Wyłączony"
#define S_SERVERWIN_STATE           "Stan: "
#define S_SERVERWIN_DONE_BUTTON     "Zrobione"
#define S_SERVERWIN_CANCEL_BUTTON   "Anuluj"
#define S_SERVERWIN_PASS_CHECK      "Użyj hasła: "

// setup window

#define S_SETUP_TITLE               "Okno Opcji"
#define S_SETUP_CONNECT_BUTTON      "Połącz"
#define S_SETUP_NETPREFS            "Opcje sieci"
#define S_SETUP_GENPREFS            "Ustawienia"
#define S_SETUP_CHOOSENET           "Wybierz sieć"
#define S_SETUP_CHOOSELABEL         "Sieć: "

// status bar

#define S_STATUS_LAG                "Opóźnienie: "
#define S_STATUS_USERS              "Użytkownicy: "
#define S_STATUS_OPS                "Operatorzy kanałów: "
#define S_STATUS_MODES              "Tryby: "
#define S_STATUS_LISTCOUNT          "Licznik: "
#define S_STATUS_LISTSTAT           "Status: "
#define S_STATUS_LISTFILTER         "Filtr: "

// window list

#define S_WINLIST_CLOSE_ITEM        "Zamknij"    

#endif _VISIONSTRINGS_H_