/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_IRC_H
#define INCLUDE_IRC_H

#include "global.h"

#include <stdarg.h>

#include "fircd.h"
#include "channel.h"
#include "network.h"
#include "array.h"

#define CRLF "\r\n"

enum irc_reply_code;

/* Enum containing all of the possible reply codes from an IRC server */
enum irc_reply_code {
    RPL_WELCOME = 1,           RPL_YOURHOST = 2,
    RPL_CREATED = 3,           RPL_MYINFO = 4,
    RPL_BOUNCE = 5,            RPL_USERHOST = 302,
    RPL_ISON = 303,            RPL_AWAY = 301,
    RPL_UNAWAY = 305,          RPL_NOWAWAY = 306,
    RPL_WHOISUSER = 311,       RPL_WHOISSERVER = 312,
    RPL_WHOISOPERATOR = 313,   RPL_WHOISIDLE = 317,
    RPL_ENDOFWHOIS = 318,      RPL_WHOISCHANNELS = 319,
    RPL_WHOWASUSER = 314,      RPL_ENDOFWHOWAS = 369,
    RPL_LISTSTART = 321,       RPL_LIST = 322,
    RPL_LISTEND = 323,         RPL_UNIQOPIS = 325,
    RPL_CHANNELMODEIS = 324,   RPL_NOTOPIC = 331,
    RPL_TOPIC = 332,           RPL_INVITING = 341,
    RPL_SUMMONING = 342,       RPL_INVITELIST = 346,
    RPL_ENDOFINVITELIST = 347, RPL_EXPECTLIST = 348,
    RPL_ENDOFEXPECTLIST = 349, RPL_VERSION = 351,
    RPL_WHOREPLY = 352,        RPL_ENDOFWHO = 315,
    RPL_NAMREPLY = 353,        RPL_ENDOFNAMES = 366,
    RPL_LINKS = 364,           RPL_ENDOFLINKS = 365,
    RPL_BANLIST = 367,         RPL_ENDOFBANLIST = 368,
    RPL_INFO = 371,            RPL_ENDOFINFO = 374,
    RPL_MOTDSTART = 375,       RPL_MOTD = 372,
    RPL_ENDOFMPTD = 367,       RPL_YOUREOPER = 381,
    RPL_REHASHING = 382,       RPL_YOURSERVICE = 383,
    RPL_TIME = 391,            RPL_USERSTART = 392,
    RPL_USERS = 393,           RPL_ENDOFUSERS = 394,
    RPL_NOUSERS = 395,         RPL_TRACELINK = 200,
    RPL_TRACECONNECTING = 201, RPL_TRACEANDSHAKE = 202,
    RPL_TRACEUNKNOWN = 203,    RPL_TRACEOPERATOR = 204,
    RPL_TRACEUSER = 205,       RPL_TRACESERVER = 206,
    RPL_TRACESERVICE = 207,    RPL_TRACENEWTYPE = 208,
    RPL_TRACECLASS = 209,      RPL_TRACECONNECT = 210,
    RPL_TRACELOG = 261,        RPL_TRACEEND = 262,
    RPL_STATSLINKINFO = 211,   RPL_STATSCOMMANDS = 212,
    RPL_ENDOFSTATS = 219,      RPL_STATSUPTIME = 242,
    RPL_STATSONLINE = 243,     RPL_UMODEIS = 221,
    RPL_SERVLIST = 234,        RPL_SERVLISTEND = 235,
    RPL_LUSERCLIENT = 251,     RPL_LUSEROP = 252,
    RPL_LUSERKNOWN = 253,      RPL_LUSERCHANNELS = 254,
    RPL_LUSERME = 255,         RPL_ADMINME = 256,
    RPL_ADMINLOC1 = 257,       RPL_ADMONLOC2 = 258,
    RPL_ADMINEMAIL = 259,      RPL_TRYAGAIN = 263,

    ERR_NOSUCHNICK = 401,       ERR_NOSUCHSERVER = 402,
    ERR_NOSUCHCHANNEL = 403,    ERR_CANNOTSENDTOCHAN = 404,
    ERR_tOOMANYCHANNELS = 405,  ERR_WASNOSUCHNICK = 406,
    ERR_TOOMANYTARGETS = 407,   ERR_NOSUCHSERVICE = 408,
    ERR_NOORIGIN = 409,         ERR_NORECIPIENT = 411,
    ERR_NOTEXTTOSEND = 412,     ERR_NOTOPLEVEL = 413,
    ERR_WILDTOPLEVEL = 414,     ERR_BADMASK = 415,
    ERR_UNKNOWNCOMMAND = 421,   ERR_NOMOTD = 422,
    ERR_NOADMININFO = 423,      ERR_FILEERROR = 424,
    ERR_NONICKNAMEGIVEN = 431,  ERR_ERRONEUSERNAME = 432,
    ERR_NICKNAMEINUSE = 433,    ERR_NICKCOLLISION = 436,
    ERR_UNAVAILRESOURCE = 437,  ERR_USERNOTINCHANNEL = 441,
    ERR_NOTONCHANNEL = 442,     ERR_USERONCHANNEL = 443,
    ERR_NOLOGIN = 444,          ERR_SUMMONDISABLED = 445,
    ERR_USERSDIABLED = 446,     ERR_NOTREGISTERED = 451,
    ERR_NEEDMOREPARAMS = 461,   ERR_ALREADYREGISTERED = 462,
    ERR_NOPERMFORHOST = 463,    ERR_PASSWDMISMATCH = 464,
    ERR_YOURBANNEDCREEP = 465,  ERR_YOUWILLBEBANNED = 466,
    ERR_KEYSET = 467,           ERR_CHANNELISFULL = 471,
    ERR_UNKNOWNMODE = 472,      ERR_INVITEONLYCHAN = 473,
    ERR_BANNEDFROMCHAN = 474,   ERR_BADCHANNELKEY = 475,
    ERR_BADCHANMASK = 476,      ERR_NOCHANMODES = 477,
    ERR_BANLISTFULL = 478,      ERR_NOPRIVILEGES = 481,
    ERR_CHANOPRIVSNEEDED = 482, ERR_CANTKILLSERVER = 483,
    ERR_RESTRICTED = 484,       ERR_UNIQOPPRIVSNEEDED = 485,
    ERR_NOOPERHOST = 491,       ERR_UMODEUNKNOWNFLAG = 501,
    ERR_USERSDONTMATCH = 502
};

struct irc_prefix {
    char *raw;
    char *user;
    char *host;
};

struct irc_reply {
    char *raw;

    struct irc_prefix prefix;

    enum irc_reply_code code;
    char *cmd;
    ARRAY(char*, lines);
    char  *colon;
};

extern struct irc_reply *irc_parse_line (const char *line);

extern void irc_reply_free (struct irc_reply *rpl);
extern void irc_send_raw   (struct network *, const char *text);
extern void irc_connect    (struct network *);
extern void irc_nick       (struct network *);
extern void irc_user       (struct network *);
extern void irc_pass       (struct network *);
extern void irc_privmsg    (struct network *, const char *chan, const char *text);
extern void irc_join       (struct network *, const char *chan);
extern void irc_part       (struct network *, const char *chan, const char *msg);
extern void irc_quit       (struct network *, const char *msg);

#endif
