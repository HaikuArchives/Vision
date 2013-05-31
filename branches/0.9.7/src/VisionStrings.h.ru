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

#ifndef _VISIONSTRINGS_H_
#define _VISIONSTRINGS_H_

// channel agent

#define S_CHANNEL_INIT              "*** Сейчас вы разговариваете на канале "
#define S_CHANNEL_REJOIN            "*** Попытка соединится заново "
#define S_CHANNEL_RECON_REJOIN      "[@] Попытка соединится заново"
#define S_CHANNEL_GOT_KICKED        "*** Вас выкинул с канала "
#define S_CHANNEL_GOT_KICKED2       " пользователь "
#define S_CHANNEL_SET_MODE          " установил режим "

// channel options

#define S_CHANOPTS_TITLE            " Свойства"
#define S_CHANOPTS_OPID1            "Вы - оператор канала. "
#define S_CHANOPTS_OPID2            "Вы можете изменить любую из этих опций."

// client agent

#define S_CLIENT_DCC_SUCCESS        "Завершено "
#define S_CLIENT_DCC_FAILED         "Остановлено "
#define S_CLIENT_DCC_SENDTYPE       "отпавлено "
#define S_CLIENT_DCC_RECVTYPE       "получено "
#define S_CLIENT_DCC_TO             " с "
#define S_CLIENT_DCC_FROM           " от "
#define S_CLIENT_DCC_SIZE_UNITS     " байт"
#define S_CLIENT_DCC_SPEED_UNITS    " символов"

// client window

#define S_CW_APP_ABOUT              "О программе"
#define S_CW_APP_PREFS              "Настройки"
#define S_CW_APP_CHANLIST           "Список каналов"
#define S_CW_APP_TERMINAL           "Новый терминал"
#define S_CW_APP_QUIT               "Выход"
#define S_CW_SERVER_MENU            "Сервер"
#define S_CW_SERVER_CONNECT         "Соединится с"
#define S_CW_SERVER_SETUP           "Установки"
#define S_CW_EDIT_MENU              "Редактировать"
#define S_CW_EDIT_CUT               "Вырезать"
#define S_CW_EDIT_COPY              "Копировать"
#define S_CW_EDIT_PASTE             "Вставить"
#define S_CW_EDIT_SELECT_ALL        "Выбрать все"
#define S_CW_WINDOW_MENU            "Окно"
#define S_CW_WINDOW_PART            "Закрыть под-окна"
#define S_CW_WINDOW_UP              "Вверх"
#define S_CW_WINDOW_DOWN            "Вниз"
#define S_CW_WINDOW_SM_UP           "Умный Вверх"
#define S_CW_WINDOW_SM_DOWN         "Умный Вниз"
#define S_CW_WINDOW_NETWORK         "Окно сетей"
#define S_CW_WINDOW_PREVIOUS        "Предыдущее окно"
#define S_CW_WINDOW_NET_UP          "На сеть вверх"
#define S_CW_WINDOW_NET_DOWN        "На сеть вниз"
#define S_CW_WINDOW_COLLAPSE        "Свернуть сеть"
#define S_CW_WINDOW_EXPAND          "Развернуть сеть"

// client window dock

#define S_CWD_WINLIST_HEADER        "Список окон"
#define S_CWD_NOTIFY_HEADER         "Список оповещений"

// DCC messages

#define S_DCC_SPEED                 "байт/с: "
#define S_DCC_ESTABLISH_ERROR       "Невозможно установить соединение."
#define S_DCC_ESTABLISH_SUCCEEDED   "Соединение установлено."
#define S_DCC_CONNECT_TO_SENDER     "Соединение с отправителем."
#define S_DCC_RECV1                 "Получаем \""
#define S_DCC_RECV2                 "\" от "
#define S_DCC_SEND1                 "Отправляем \""
#define S_DCC_SEND2                 "\" к "
#define S_DCC_LOCK_ACQUIRE          "Acquiring DCC lock"
#define S_DCC_ACK_WAIT              "Waiting for acceptance"
#define S_DCC_LISTEN_CALL           "Doing listen call."
#define S_DCC_WAIT_FOR_CONNECTION   "Ожидаем соединение "
#define S_DCC_WRITE_ERROR           "Ошибка записи данных."
#define S_DCC_SOCKET_ERROR          "Ошибка создания сокета. \n"
#define S_DCC_BIND_ERROR            "Ошибка привязки сокета.\n"
#define S_DCC_CONN_ERROR            "Ошибка соединения сокета.\n"
#define S_DCC_CHAT_LISTEN           "Соединяемся по адресу "
#define S_DCC_CHAT_TRY              "Пытаемся соединится по адресу "
#define S_DCC_CHAT_PORT             ", порт "
#define S_DCC_CHAT_CONNECTED        "Соединились!\n"
#define S_DCC_CHAT_TERM             "DCC-чат остановлен.\n"

// list agent

#define S_LIST_MENU                 "Каналы"
#define S_LIST_MENU_FIND            "Найти"
#define S_LIST_MENU_FINDNEXT        "Найти следующий"
#define S_LIST_MENU_FILTER          "Фильтр"
#define S_LIST_COLUMN_CHAN          "Канал"
#define S_LIST_COLUMN_USER          "Пользователи"
#define S_LIST_COLUMN_TOPIC         "Тема"
#define S_LIST_STATUS_LOADING       "Загружается"
#define S_LIST_STATUS_DONE          "Готово"
#define S_LIST_PROMPT_TITLE         "Найти"
#define S_LIST_PROMPT_LABEL         "    Найти:"

// network prefs -- server list

#define S_PREFSERVER_STATUS_COLUMN  "Статус"
#define S_PREFSERVER_SERVER_COLUMN  "Сервер"
#define S_PREFSERVER_PORT_COLUMN    "Порт"
#define S_PREFSERVER_ADD_BUTTON     "Добавить"
#define S_PREFSERVER_REMOVE_BUTTON  "Удалить"
#define S_PREFSERVER_EDIT_BUTTON    "Редактировать"
#define S_PREFSERVER_DESC1          "Ключ: "
#define S_PREFSERVER_DESC2          "  * = основной"
#define S_PREFSERVER_DESC3          "  + = запасной (резервный)"
#define S_PREFSERVER_DESC4          "  - = отключен"
#define S_PREFSERVER_OK_BUTTON      "OK"
#define S_PREFSERVER_SEL_STRING     "Выберите сервер "

// network prefs -- main view

#define S_NETPREFS_NETMENU          "Сети"
#define S_NETPREFS_DEFAULTS_ITEM    "По умолчанию"
#define S_NETPREFS_ADD_NEW          "Добавить новый"
#define S_NETPREFS_REMOVE           "Удалить текущий"
#define S_NETPREFS_DUPE             "Скопировать текущий"
#define S_NETPREFS_NET_BOX          "Настройки сети"
#define S_NETPREFS_PERSONAL_BOX     "Личные настройки"
#define S_NETPREFS_CONN1            "Содинятся с"
#define S_NETPREFS_CONN2            "возвращаемся к 9-ти другим."
#define S_NETPREFS_CHANGE_SERVER    "Сменить сервера"
#define S_NETPREFS_AUTOEXEC         "Выполнить:"
#define S_NETPREFS_LAG_CHECK        "Включить проверку лагов"
#define S_NETPREFS_STARTUP_CONN     "Соединятся с этой сетью при старте Vision "
#define S_NETPREFS_USE_DEFAULTS     "Использовать по умолчанию"
#define S_NETPREFS_PREFNICK         "Предпочитаемые псевдонимы:"
#define S_NETPREFS_ADD_BUTTON       "Добавить"
#define S_NETPREFS_REMOVE_BUTTON    "Удалить"
#define S_NETPREFS_IDENT            "Ident: "
#define S_NETPREFS_REALNAME         "Настоящее имя "
#define S_NETPREFS_FALLBACK1        "возвращаемся к  "
#define S_NETPREFS_FALLBACK2        " другим"
#define S_NETPREFS_FALLBACK2_PLURAL ""
#define S_NETPREFS_ADDNET_TITLE     "Добавить сеть"
#define S_NETPREFS_DUPENET_TITLE    "Скопировать сеть"
#define S_NETPREFS_NET_PROMPT       "Название сети: "
#define S_NETPREFS_ADDNICK_TITLE    "Добавить всевдоним"
#define S_NETPREFS_ADDNICK_PROMPT   "Псевдоним: "

// network windows

#define S_NETWORK_WINDOW_TITLE      "Настройки сети"
#define S_SERVERPREFS_TITLE         "Сервера"

// notify list

#define S_NOTIFYLIST_QUERY_ITEM     "Запрос"
#define S_NOTIFYLIST_WHOIS_ITEM     "Инфо"
#define S_NOTIFYLIST_REMOVE_ITEM    "Удалить"
#define S_NOTIFYLIST_DCC_ITEM       "DCC-чат"
// command parser

#define S_PCMD_PARAMETER_ERROR      "Ошибка: Неверные параметры"
#define S_PCMD_SEND_TITLE           "Отправить файл для "
#define S_PCMD_SEND_BUTTON          "Отправить"
#define S_PCMD_TRY_NEW_NICK         "*** Пробуем псевдоним "
#define S_PCMD_SET_BOOL_SUCCESS     "Bool has been set"
#define S_PCMD_SET_BOOL_FAILURE     "Error setting bool"
#define S_PCMD_DNS1                 "Соединяемся  "
#define S_PCMD_DNS2                 " с "
#define S_PCMD_DNS_ERROR            "Невозможно соединится "
#define S_PCMD_PEXEC_ERROR          "/pexec: команда не сработала"
#define S_PCMD_VIS_UPTIME           "Vision работает "
// ctcp parser

#define S_PCTCP_RESPONSE            " ответ"
#define S_PCTCP_SECOND              "секунда"
#define S_PCTCP_SECOND_PLURAL       "секунды"

// numerics parser

#define S_PENUM_CURMODE             "[x] ваш нынешний режим: "
#define S_PENUM_WHOWAS              "[был]"
#define S_PENUM_IDLE                "Ожидание: "
#define S_PENUM_SIGNON              "Подписка: "
#define S_PENUM_WHOIS_CHANNELS      "[x] Каналы: "
#define S_PENUM_CHANMODE            "*** Режим канала для "
#define S_PENUM_MLOCK               "*** Блокировка канала для "
#define S_PENUM_CHANCREATED         "создано"
#define S_PENUM_NO_TOPIC            "[x] Не задан топик на канале "
#define S_PENUM_DISP_TOPIC          "*** Тема: "
#define S_PENUM_TOPIC_SET_BY        "*** Тема установлена пользователем "
#define S_PENUM_INVITING            " приглашен на канал "
#define S_PENUM_NAMEREPLY           "*** Пользователи на канале "
#define S_PENUM_SERVER_MOTD         "- Сообщения Дня сервера:"
#define S_PENUM_RECON_SUCCESS       "[@] Успешно пересоединились"
#define S_PENUM_NICKINUSE1          "* Псевдоним \""
#define S_PENUM_NICKINUSE2          "\" используется или недоступен.. пытаемся \""
#define S_PENUM_NICKINUSE3          "[x] Псевдоним/Канал "
#define S_PENUM_NICKINUSE4          " уже используется или недоступно."
#define S_PENUM_ALLNICKSUSED1       "* Все ваши заданные псевдонимы - уже используюся."
#define S_PENUM_ALLNICKSUSED2       "* Пожалуйста наберите /NICK <NEWNICK> чтобы попробовать лругой псевдоним."
#define S_PENUM_NOTINCHANNEL        " не в  "
#define S_PENUM_ME_NOTINCHANNEL     "[x] Вы не на канале "
#define S_PENUM_ALREADYINCHANNEL    " уже на канале "
#define S_PENUM_KEY_ALREADY_SET     "[x] Ключ канала уже используется "
#define S_PENUM_UNKNOWNCHANMODE     "[x] Неизвестный режим канала: '"
#define S_PENUM_INVITE_ONLY         " (только по приглашенгию)"
#define S_PENUM_BANNED              " (вас забанили)"
#define S_PENUM_BADCHANKEY          " (плохой ключ канала)"
#define S_PENUM_UNKNOWNMODE         "[x] Неизвестный MODE-флаг."

// events parser

#define S_PEVENTS_UMODE_CHANGE      "*** Режим пользователя изменен: "
#define S_PEVENTS_INVITE1           "*** Вас пригласили на канал "
#define S_PEVENTS_INVITE2           " пригласил -  "
#define S_PEVENTS_SILENCE_ADDED     "*** Добавлены в список МОЛЧУНОВ: "
#define S_PEVENTS_SILENCE_REMOVED   "*** Удалены из списка МОЛЧУНОВ: "

// app prefs

#define S_PREFAPP_VERSION_PARANOID  "Показывать информацию об ОС в ответе на запрос версии"
#define S_PREFAPP_CMDW              "Требуется дважды нажать Cmd+Q/W для выхода"
#define S_PREFAPP_STRIP_MIRC        "Убрать цвета mIRC"
#define S_PREFAPP_WARN_MULTILINE    "Предупреждение, когда вставляется более одной строки"
#define S_PREFAPP_QUERY_MSG         "Открыть новые окно при сообщении"

// color prefs

#define S_PREFCOLOR_TEXT            "Текст"
#define S_PREFCOLOR_BACKGROUND      "Фон"
#define S_PREFCOLOR_URL             "Ссылка"
#define S_PREFCOLOR_SERVERTEXT      "Текст сервера"
#define S_PREFCOLOR_NOTICE          "Замечание"
#define S_PREFCOLOR_ACTION          "Действие"
#define S_PREFCOLOR_QUIT            "Выход"
#define S_PREFCOLOR_ERROR           "Ошибка"
#define S_PREFCOLOR_NICK_EDGES      "Обрамления псевдонимов"
#define S_PREFCOLOR_UNICK_EDGES     "Пользовательские обормления псевдонимов"
#define S_PREFCOLOR_NICK_TEXT       "Текст псевдонима"
#define S_PREFCOLOR_JOIN            "Соединится"
#define S_PREFCOLOR_KICK            "Выкинуть"
#define S_PREFCOLOR_WHOIS           "Инфо"
#define S_PREFCOLOR_NAMES_NORM      "Имена (обычный)"
#define S_PREFCOLOR_NAMES_OP        "Имена (Оператор)"
#define S_PREFCOLOR_NAMES_HELP      "Имена (Helper)"
#define S_PREFCOLOR_NAMES_VOICE     "Имена (с голосом)"
#define S_PREFCOLOR_NAMES_SEL       "Выбранные имена"
#define S_PREFCOLOR_NAMES_BG        "Фон имен"
#define S_PREFCOLOR_CTCP_REQ        "CTCP запрос"
#define S_PREFCOLOR_CTCP_RPY        "CTCP ответ"
#define S_PREFCOLOR_IGNORE          "Игнор"
#define S_PREFCOLOR_INPUT_TXT       "Введенный текст"
#define S_PREFCOLOR_INPUT_BG        "Фон текста"
#define S_PREFCOLOR_WLIST_NORM      "Winlist normal status"
#define S_PREFCOLOR_WLIST_TXT       "Winlist text status"
#define S_PREFCOLOR_WLIST_NICK      "Winlist nick alert status"
#define S_PREFCOLOR_WLIST_SEL       "Winlist selection status"
#define S_PREFCOLOR_WLIST_EVT       "Winlist event status"
#define S_PREFCOLOR_WLIST_BG        "Winlist background"
#define S_PREFCOLOR_WALLOPS         "Wallops"
#define S_PREFCOLOR_TIMESTAMP       "Время"
#define S_PREFCOLOR_TIMESTAMP_BG    "Фон времени"
#define S_PREFCOLOR_SELECTION       "Выбор"
#define S_PREFCOLOR_MIRCWHITE       "mIRC Белый"
#define S_PREFCOLOR_MIRCBLACK       "mIRC Черный"
#define S_PREFCOLOR_MIRCDBLUE       "mIRC Темно-синий"
#define S_PREFCOLOR_MIRCGREEN       "mIRC Зеленый"
#define S_PREFCOLOR_MIRCRED         "mIRC Красный"
#define S_PREFCOLOR_MIRCBROWN       "mIRC Коричневый"
#define S_PREFCOLOR_MIRCPURPLE      "mIRC Пурпурный"
#define S_PREFCOLOR_MIRCORANGE      "mIRC Оранжевый"      
#define S_PREFCOLOR_MIRCYELLOW      "mIRC Желтый"
#define S_PREFCOLOR_MIRCLIME        "mIRC Lime"
#define S_PREFCOLOR_MIRCTEAL        "mIRC Teal"
#define S_PREFCOLOR_MIRCAQUA        "mIRC Aqua"
#define S_PREFCOLOR_MIRCLBLUE       "mIRC Светло-голубой"
#define S_PREFCOLOR_MIRCPINK        "mIRC Розовый"
#define S_PREFCOLOR_MIRCGREY        "mIRC Серый"
#define S_PREFCOLOR_MIRCSILVER      "mIRC Серебрянный"
#define S_PREFCOLOR_NOTIFY_ON       "Оповещать Notify Online"
#define S_PREFCOLOR_NOTIFY_OFF      "Оповещать Offline"
#define S_PREFCOLOR_NOTIFY_BG       "Оповещать List background"
#define S_PREFCOLOR_NOTIFY_SEL      "Оповещать List selection"
#define S_PREFCOLOR_REVERT          "Реверсировать"

// command prefs

#define S_PREFCOMMAND_QUIT          "Выход:"
#define S_PREFCOMMAND_KICK          "Выкинуть:"
#define S_PREFCOMMAND_IGNORE        "Игнорировать:"
#define S_PREFCOMMAND_UNIGNORE      "Разигнорировать:"
#define S_PREFCOMMAND_AWAY          "Отошел:"
#define S_PREFCOMMAND_BACK          "Вернулся:"
#define S_PREFCOMMAND_UPTIME        "Время работы:"

// dcc prefs

#define S_PREFDCC_BLOCK_SIZE        "Размер блока DCC: "
#define S_PREFDCC_AUTOACK           "Автоматически разрешать входящие файлы"
#define S_PREFDCC_PRIVATE           "Автоматически проверять на NAT IP"
#define S_PREFDCC_DEFPATH           "Путь по умолчанию: "
#define S_PREFDCC_PORTRANGE         "Диапазон портов DCC"
#define S_PREFDCC_PORTMIN           "Минимальный: "
#define S_PREFDCC_PORTMAX           "Максимальный: "

// event prefs

#define S_PREFEVENT_JOIN            "Соединится:"
#define S_PREFEVENT_PART            "Часть:"
#define S_PREFEVENT_NICK            "Псевдоним:"
#define S_PREFEVENT_QUIT            "Выход:"
#define S_PREFEVENT_KICK            "Выкинули:"
#define S_PREFEVENT_TOPIC           "Тема:"
#define S_PREFEVENT_SNOTICE         "Сообщение сервера:"
#define S_PREFEVENT_UNOTICE         "Сообщение пользователя:"
#define S_PREFEVENT_NOTIFYON        "Оповещения включены:"
#define S_PREFEVENT_NOTIFYOFF       "Оповещения выключены:"

// font prefs

#define S_PREFFONT_TEXT             "Текст"
#define S_PREFFONT_SMESSAGES        "Сообщения сервера"
#define S_PREFFONT_URLS             "Ссылки"
#define S_PREFFONT_NAMESLIST        "Список имен"
#define S_PREFFONT_INPUT_TEXT       "Текст ввода"
#define S_PREFFONT_WINLIST          "Список окон"
#define S_PREFFONT_CHANLIST         "Список каналов"
#define S_PREFFONT_TSTAMP           "Время"
#define S_PREFFONT_FONTLABEL        "Шрифт: "
#define S_PREFFONT_SIZELABEL        "Размер: "

// log prefs

#define S_PREFLOG_LOGPATH           "Путь к логам:"
#define S_PREFLOG_TS_FORMAT         "Формат времени:"
#define S_PREFLOG_SHOW_TIMESTAMP    "Показывать время в окне IRC"
#define S_PREFLOG_USE_LOGGING       "Лог включен"
#define S_PREFLOG_LOG_TIMESTAMP     "Добавлять время в имя файлов лога"
#define S_PREFLOG_ALERT_TITLE       "Ошибка"
#define S_PREFLOG_ALERT_TEXT        "Путь к логам, который вы ввели - неправильный."
#define S_PREFLOG_ALERT_BUTTON      "OK"

// main prefs view

#define S_PREFGEN_APP_ITEM          "Приложение"
#define S_PREFGEN_COLOR_ITEM        "Цвета"
#define S_PREFGEN_FONT_ITEM         "Шрифты"
#define S_PREFGEN_COMMAND_ITEM      "Команды"
#define S_PREFGEN_EVENT_ITEM        "События"
#define S_PREFGEN_DCC_ITEM          "DCC"
#define S_PREFGEN_LOG_ITEM          "Лог"

// preferences window

#define S_PREFSWIN_TITLE            "Настройки"

// server agent

#define S_SERVER_ATTEMPT1           "[@] Пытаемся "
#define S_SERVER_ATTEMPT2           "пере"
#define S_SERVER_ATTEMPT3           "соединится (попытка "
#define S_SERVER_ATTEMPT4           " из "
#define S_SERVER_ATTEMPT5           "[@] Попытка соединится с "
#define S_SERVER_CONN_ERROR1        "[@] Не могу соединится по этому адресу и порту. Убедитесь, что ваше соединение с Интернетом функционирует нормально."
#define S_SERVER_CONN_ERROR2        "[@] Не могу установить соединение с сервером."
#define S_SERVER_CONN_OPEN          "[@] Соединение установлено, ожидаем ответа от сервера"
#define S_SERVER_LOCALIP_ERROR      "[@] Ошибка при получении локального IP"
#define S_SERVER_LOCALIP            "[@] Локальный IP: "
#define S_SERVER_PROXY_MSG          "[@] (Вы соединяетесь через прокси-сервер. Vision будет опрашивать IRC-server до тех пор, пока он успешно не соединится с вами. Это соединение будет использоваться для DCC.)"
#define S_SERVER_PASS_MSG           "[@] Отправляем пароль"
#define S_SERVER_HANDSHAKE          "[@] Настройка"
#define S_SERVER_ESTABLISH          "[@] Установлено"
#define S_SERVER_RETRY_LIMIT        "[@] Лимит переподключений достигнут; сдаюсь. Наберите /reconnect если вы хотите попробовать еще раз."
#define S_SERVER_DISCONNECT         "[@] Отключен от "
#define S_SERVER_DISCON_STATUS      "Отключен"
#define S_SERVER_CONN_PROBLEM       "ПРОБЛЕМЫ СОЕДИНЕНИЯ"
#define S_SERVER_LAG_DISABLED       "Отключен"
#define S_SERVER_DCC_CHAT_PROMPT    " хочет начать DCC-чат с вами."
#define S_SERVER_WAITING_RETRY      "[@] Ожидаем "
#define S_SERVER_WAITING_SECONDS    " секунд"
#define S_SERVER_WAITING_PLURAL     ""
#define S_SERVER_WAITING_ENDING     " до следующей попытки"

// server entry window

#define S_SERVERWIN_TITLE           "Добавить сервер"
#define S_SERVERWIN_SERVER          "Сервер: "
#define S_SERVERWIN_PORT            "Порт: "
#define S_SERVERWIN_MENU1           "Выберите статус"
#define S_SERVERWIN_MENU_PRI        "Основной"
#define S_SERVERWIN_MENU_SEC        "Запасной"
#define S_SERVERWIN_MENU_DIS        "Отключен"
#define S_SERVERWIN_STATE           "Состояние: "
#define S_SERVERWIN_DONE_BUTTON     "Готово"
#define S_SERVERWIN_CANCEL_BUTTON   "Отмена"
#define S_SERVERWIN_PASS_CHECK      "Использовать пароль: "

// setup window

#define S_SETUP_TITLE               "Окно Установок"
#define S_SETUP_CONNECT_BUTTON      "Соединится"
#define S_SETUP_NETPREFS            "Настройка сети"
#define S_SETUP_GENPREFS            "Настройки"
#define S_SETUP_CHOOSENET           "Выберете сеть"
#define S_SETUP_CHOOSELABEL         "Сеть: "

// status bar

#define S_STATUS_LAG                "Задержка: "
#define S_STATUS_USERS              "Пользователи: "
#define S_STATUS_OPS                "Операторы: "
#define S_STATUS_MODES              "Режимы: "
#define S_STATUS_LISTCOUNT          "Всего: "
#define S_STATUS_LISTSTAT           "Статус: "
#define S_STATUS_LISTFILTER         "Фильтр: "

// window list

#define S_WINLIST_CLOSE_ITEM        "Закрыть"	

#endif _VISIONSTRINGS_H_
