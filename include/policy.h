/*
 *  Project   : tin - a Usenet reader
 *  Module    : policy.h
 *  Author    : Ralf Doeblitz <doeblitz@gmx.de>
 *  Created   : 1999-01-12
 *  Updated   : 2015-09-14
 *  Notes     : #defines and static data for policy configuration
 *
 * Copyright (c) 1999-2021 Ralf Doeblitz <doeblitz@gmx.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * CAUTION: THIS FILE IS OUTDATED AND NO LONGER ACTIVELY MAINTAINED
 * "disable_gnksa_domain_check=OFF" MIGHT RESULT IN FALSE POSITIVES
 */

/*
 * tables for TLD search in misc.c, gnksa domain checking
 */

#ifndef TIN_POLICY_H
#	define TIN_POLICY_H 1
/*
 * known two letter country codes
 *
 * .ac  Ascension Island               .ad  Andorra
 * .ae  United Arab Emirates           .af  Afghanistan
 * .ag  Antigua and Barbuda            .ai  Anguilla
 * .al  Albania                        .am  Armenia
 * .an  Netherlands Antilles           .ao  Angola
 * .aq  Antarctica                     .ar  Argentina
 * .as  American Samoa                 .at  Austria
 * .au  Australia                      .aw  Aruba
 * .ax  Aland                          .az  Azerbaijan
 *
 * .ba  Bosnia and Herzegowina         .bb  Barbados
 * .bd  Bangladesh                     .be  Belgium
 * .bf  Burkina Faso                   .bg  Bulgaria
 * .bh  Bahrain                        .bi  Burundi
 * .bj  Benin                          .bm  Bermuda
 * .bn  Brunei Darussalam              .bo  Bolivia
 * .br  Brazil                         .bs  Bahamas
 * .bt  Bhutan                         .bw  Botswana
 * .by  Belarus                        .bz  Belize
 *
 * .ca  Canada                         .cc  Cocos (Keeling) Islands
 * .cd  Congo, Democratic People's Republic
 * .cf  Central African Republic       .cg  Congo, Republic of
 * .ch  Switzerland                    .ci  Côte d'Ivoire
 * .ck  Cook Islands                   .cl  Chile
 * .cm  Cameroon                       .cn  China
 * .co  Colombia                       .cr  Costa Rica
 * .cu  Cuba                           .cv  Cape Verde
 * .cw  Curacao                        .cx  Christmas Island
 * .cy  Cyprus                         .cz  Czech Republic
 *
 * .de  Germany                        .dj  Djibouti
 * .dk  Denmark                        .dm  Dominica
 * .do  Dominican Republic             .dz  Algeria
 *
 * .ec  Ecuador                        .ee  Estonia
 * .eg  Egypt                          .er  Eritrea
 * .es  Spain                          .et  Ethiopia
 * .eu  European Union
 *
 * .fi  Finland                        .fj  Fiji
 * .fk  Falkland Islands (Malvina)     .fm  Micronesia, Federal State of
 * .fo  Faroe Islands                  .fr  France
 *
 * .ga  Gabon                          .gb  United Kingdom
 * .gd  Grenada                        .ge  Georgia
 * .gf  French Guiana                  .gg  Guernsey
 * .gh  Ghana                          .gi  Gibraltar
 * .gl  Greenland                      .gm  Gambia
 * .gn  Guinea                         .gp  Guadelope
 * .gq  Equatorial Guinea              .gr  Greece
 * .gs  South Georgia and the South Sandwich Islands
 * .gt  Guatemala                      .gu  Guam
 * .gw  Guinea-Bissau                  .gy  Guyana
 *
 * .hk  Hong Kong                      .hm  Heard and McDonald Islands
 * .hn  Honduras                       .hr  Croatia/Hrvatska
 * .ht  Haiti                          .hu  Hungary
 *
 * .id  Indonesia                      .ie  Ireland
 * .il  Israel                         .im  Isle of Man
 * .in  India                          .io  British Indian Ocean Territory
 * .iq  Iraq                           .ir  Iran
 * .is  Iceland                        .it  Italy
 *
 * .je  Jersey                         .jm  Jamaica
 * .jo  Jordan                         .jp  Japan
 *
 * .ke  Kenya                          .kg  Kyrgystan
 * .kh  Cambodia                       .ki  Kiribati
 * .km  Comoros                        .kn  Saint Kitts and Nevis
 * .kp  Korea, Democratic People's Republic
 * .kr  Korea, Republic of             .kw  Kuwait
 * .ky  Cayman Islands                 .kz  Kazakhstan
 *
 * .la  Laos (People's Democratic Republic)
 * .lb  Lebanon                        .lc  Saint Lucia
 * .li  Liechtenstein                  .lk  Sri Lanka
 * .lr  Liberia                        .ls  Lesotho
 * .lt  Lithuania                      .lu  Luxembourg
 * .lv  Latvia                         .ly  Libyan Arab Jamahiriya
 *
 * .ma  Morocco                        .mc  Monaco
 * .md  Moldova, Republic of           .me  Montenegro
 * .mg  Madagascar                     .mh  Marshall Islands
 * .mk  Macedonia                      .ml  Mali
 * .mm  Myanmar                        .mn  Mongolia
 * .mo  Macau                          .mp  Northern Mariana Islands
 * .mq  Martinique                     .mr  Mauritania
 * .ms  Montserrat                     .mt  Malta
 * .mu  Mauritius                      .mv  Maldives
 * .mw  Malawi                         .mx  Mexico
 * .my  Malaysia                       .mz  Mozambique
 *
 * .na  Namibia                        .nc  New Caledonia
 * .ne  Niger                          .nf  Norfolk Island
 * .ng  Nigeria                        .ni  Nicaragua
 * .nl  The Netherlands                .no  Norway
 * .np  Nepal                          .nr  Nauru
 * .nu  Niue                           .nz  New Zealand
 *
 * .om  Oman
 *
 * .pa  Panama                         .pe  Peru
 * .pf  French Polynesia               .pg  Papua New Guinea
 * .ph  Philippines                    .pk  Pakistan
 * .pl  Poland                         .pm  St. Pierre and Miquelon
 * .pn  Pitcairn Island                .pr  Puerto Rico
 * .ps  Palestinian Territories        .pt  Portugal
 * .pw  Palau                          .py  Paraguay
 *
 * .qa  Qatar
 *
 * .re  Reunion                        .ro  Romania
 * .rs  Republic of Serbia             .ru  Russian Federation
 * .rw  Rwanda
 *
 * .sa  Saudi Arabia                   .sb  Solomon Islands
 * .sc  Seychelles                     .sd  Sudan
 * .se  Sweden                         .sg  Singapore
 * .sh  St. Helena                     .si  Slovenia
 * .sk  Slovakia                       .sl  Sierra Leone
 * .sm  San Marino                     .sn  Senegal
 * .so  Somalia                        .sr  Surinam
 * .st  Sao Tome and Principe          .su  Soviet Union (former)
 * .sv  El Salvador                    .sx  Sint Maarten (Dutch side)
 * .sy  Syrian Arab Republic           .sz  Swaziland
 *
 * .tc  The Turks & Caicos Islands     .td  Chad
 * .tf  French Southern Territories    .tg  Togo
 * .th  Thailand                       .tj  Tajikistan
 * .tk  Tokelau                        .tl  Timor-Leste
 * .tm  Turkmenistan                   .tn  Tunisia
 * .to  Tonga                          .tp  East Timor
 * .tr  Turkey                         .tt  Trinidad and Tobago
 * .tv  Tuvalu                         .tw  Taiwan
 * .tz  Tanzania
 *
 * .ua  Ukraine                        .ug  Uganda
 * .uk  United Kingdom
 * .us  United States                  .uy  Uruguay
 * .uz  Uzbekistan
 *
 * .va  Holy See (Vatican City State)  .vc  Saint Vincent and the Grenadines
 * .ve  Venezuela                      .vg  Virgin Islands (British)
 * .vi  Virgin Islands (U.S)           .vn  Vietnam
 * .vu  Vanuatu
 *
 * .wf  Wallis and Futuna Islands      .ws  Western Samoa
 *
 * .ye  Yemen                          .yt  Mayotte
 *
 * .za  South Africa                   .zm  Zambia
 * .zw  Zimbabwe
 *
 *
 * invalid/obsolete TLDs:
 * .bu  Burma, now .mm
 * .cs  former Czechoslovakia, now .cz and .sk
 * .dd  former German Democratic Republic, now .de
 * .fx  France, Metropolitan
 * .oz  Australian MHSnet
 * .um  United States Minor Outlying Islands
 * .yd  Democratic Yemen, now .ye
 * .yu  Yugoslavia, now .rs and .me
 * .wg  West Bank and Gaza, now .ps
 * .zr  former Zaire, now .cd
 *
 * user-assigned code elements:
 * .aa, .qm, .qz, .xa, .xz, .zz
 *
 * unused TLDs:
 * .bl  Saint Barthélemy
 * .bq  Bonaire, Sint Eustatius and Saba
 * .bv  Bouvet Island (Norway)
 * .eh  Western Sahara
 * .gb  United Kingdom, use .uk
 * .mf  Sint Maarten (French side)
 * .sj  Svalbard and Jan Mayen Islands (Norway)
 * .ss  South Sudan
 *
 * requested new TLDs:
 *
 * xccTLDs:
 * .an  Netherlands Antilles, becomes .bq, .cw, and .sx
 * .su  former USSR, now .ru
 * .tp  former East Timor, now .tl
 */

static char gnksa_country_codes[26*26] = {
/*      A B C D E  F G H I J  K L M N O  P Q R S T  U V W X Y Z */
/* A */ 0,0,1,1,1, 1,1,0,1,0, 0,1,1,1,1, 0,1,1,1,1, 1,0,1,1,0,1,
/* B */ 1,1,0,1,1, 1,1,1,1,1, 0,0,1,1,1, 0,0,1,1,1, 0,0,1,0,1,1,
/* C */ 1,0,1,1,0, 1,1,1,1,0, 1,1,1,1,1, 0,0,1,0,0, 1,1,1,1,1,1,
/* D */ 0,0,0,0,1, 0,0,0,0,1, 1,0,1,0,1, 0,0,0,0,0, 0,0,0,0,0,1,
/* E */ 0,0,1,0,1, 0,1,0,0,0, 0,0,0,0,0, 0,0,1,1,1, 1,0,0,0,0,0,
/* F */ 0,0,0,0,0, 0,0,0,1,1, 1,0,1,0,1, 0,0,1,0,0, 0,0,0,0,0,0,
/* G */ 1,0,0,1,1, 1,1,1,1,0, 0,1,1,1,0, 1,1,1,1,1, 1,0,1,0,1,0,
/* H */ 0,0,0,0,0, 0,0,0,0,0, 1,0,1,1,0, 0,0,1,0,1, 1,0,0,0,0,0,
/* I */ 0,0,0,1,1, 0,0,0,0,0, 0,1,1,1,1, 0,1,1,1,1, 0,0,0,0,0,0,
/* J */ 0,0,0,0,1, 0,0,0,0,0, 0,0,1,0,1, 1,0,0,0,0, 0,0,0,0,0,0,
/* K */ 0,0,0,0,1, 0,1,1,1,0, 0,0,1,1,0, 1,0,1,0,0, 0,0,1,0,1,1,
/* L */ 1,1,1,0,0, 0,0,0,1,0, 1,0,0,0,0, 0,0,1,1,1, 1,1,0,0,1,0,
/* M */ 1,0,1,1,1, 0,1,1,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1,1,
/* N */ 1,0,1,0,1, 1,1,0,1,0, 0,1,0,0,1, 1,0,1,0,0, 1,0,0,0,0,1,
/* O */ 0,0,0,0,0, 0,0,0,0,0, 0,0,1,0,0, 0,0,0,0,0, 0,0,0,0,0,0,
/* P */ 1,0,0,0,1, 1,1,1,0,0, 1,1,1,1,0, 0,0,1,1,1, 0,0,1,0,1,0,
/* Q */ 1,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,0,
/* R */ 0,0,0,0,1, 0,0,0,0,0, 0,0,0,0,1, 0,0,0,1,0, 1,0,1,0,0,0,
/* S */ 1,1,1,1,1, 0,1,1,1,0, 1,1,1,1,1, 0,0,1,0,1, 1,1,0,1,1,1,
/* T */ 0,0,1,1,0, 1,1,1,0,1, 1,1,1,1,1, 1,0,1,0,1, 0,1,1,0,0,1,
/* U */ 1,0,0,0,0, 0,1,0,0,0, 1,0,0,0,0, 0,0,0,1,0, 0,0,0,0,1,1,
/* V */ 1,0,1,0,1, 0,1,0,1,0, 0,0,0,1,0, 0,0,0,0,0, 1,0,0,0,0,0,
/* W */ 0,0,0,0,0, 1,0,0,0,0, 0,0,0,0,0, 0,0,0,1,0, 0,0,0,0,0,0,
/* X */ 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,0,
/* Y */ 0,0,0,0,1, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,1, 0,0,0,0,0,0,
/* Z */ 1,0,0,0,0, 0,0,0,0,0, 0,0,1,0,0, 0,0,0,0,0, 0,0,1,0,0,0
/*      A B C D E  F G H I J  K L M N O  P Q R S T  U V W X Y Z */
};


/*
 * valid domains with 3 or more characters
 */
static const char *gnksa_domain_list[] = {
	"biz",	/* Businesses */
	"cat",	/* Catalan linguistic and cultural community */
	"com",	/* Commercial */
	"edu",	/* Educational */
	"gov",	/* US Government */
	"int",	/* International Organizations */
	"mil",	/* US Dept of Defense */
	"net",	/* Networks */
	"org",	/* Organizations */
	"pro",	/* Accountants, lawyers, and physicians */
	"tel",	/* Internet communication */
	"xxx",	/* For adult entertainment */
	"aero",	/* Air-transport industry */
	"arpa",	/* Address and Routing Parameter Area */
	"asia",	/* Asia and the Pacific */
	"bike",
	"coop",	/* Non-profit cooperatives */
	"guru",
	"info",	/* Unrestricted use */
	"jobs",	/* Human resource managers */
	"land",
	"mobi",	/* Mobile products and services */
	"name",	/* For registration by individuals */
	"post",	/* Postal sector */
	"sexy",
	"camera",
	"estate",
	"museum",	/* Museums */
	"tattoo",
	"travel",	/* Travel industry */
	"voyage",
	"gallery",
	"singles",
	"clothing",
	"graphics",
	"holdings",
	"lighting",
	"plumbing",
	"ventures",
	"equipment",
	"technology",
	"contractors",
	"construction",
	/*
	 * more gTLDs to come, proposed are:
	 *  .kids     .mail     .catholic
	 *  (.berlin) (.sco)    (.bzh)    (.cym)    (.gal)
	 */
#	if 0		/* $DEAD */
	"nato",
	"uucp",
	"csnet",
	"bitnet",
#	endif /* 0 */
	/* the next five are defined in RFC 2606, RFC 6761 */
	"invalid",
#	if 0
	/* but four of them shouldn't be used on usenet */
	"test",
	"onion",			/* RFC 7686 */
	"example",
	"localhost",
#	endif /* 0 */
#	ifdef TINC_DNS
	"bofh",	/* There Is No Cabal */
#	endif /* TINC_DNS */
	/* active IDN ccTLDs */
	"xn--lgbbat1ad8j",	/* Algeria */
	"xn--fiqs8s",		/* China, Simplified Chinese */
	"xn--fiqz9s",		/* China, Traditional Chinese */
	"xn--wgbh1c",		/* Egypt */
	"xn--j6w193g",		/* Hong Kong, Han */
	"xn--45brj9c",		/* India, Bengali */
	"xn--gecrj9c",		/* India, Gujarati */
	"xn--h2brj9c",		/* India, Hindi */
	"xn--s9brj9c",		/* India, Punjabi */
	"xn--xkc2dl3a5ee0h",	/* India, Tamil */
	"xn--fpcrj9c3d",	/* India, Telugu */
	"xn--mgbbh1a71e",	/* India, Urdu */
	"xn--mgba3a4f16a",  /* Iran */
	"xn--mgbayh7gpa",	/* Jordan */
	"xn--80ao21a",		/* Kazakhstan */
	"xn--3e0b707e",		/* Korea, Republic of */
	"xn--mgbx4cd0ab",	/* Malaysia */
	"xn--l1acc",		/* Mongolia */
	"xn--mgbc0a9azcg",	/* Morocco */
	"xn--mgb9awbf",		/* Oman */
	"xn--ygbi2ammx",	/* Palestinian Territory */
	"xn--wgbl6a",		/* Qatar */
	"xn--p1ai",			/* Russian Federation */
	"xn--mgberp4a5d4ar",	/* Saudi Arabia */
	"xn--90a3ac",		/* Serbia */
	"xn--yfro4i67o",	/* Singapore, Chinese */
	"xn--clchc0ea0b2g2a9gcd",	/* Singapore, Tamil */
	"xn--fzc2c9e2c",	/* Sri Lanka, Sinhala */
	"xn--xkc2al3hye2a",	/* Sri Lanka, Tamil */
	"xn--ogbpf8fl",		/* Syrian Arab Republic */
	"xn--kpry57d",		/* Taiwan, Simplified Chinese */
	"xn--kprw13d",		/* Taiwan, Traditional Chinese */
	"xn--o3cw4h",		/* Thailand */
	"xn--pgbs0dh",		/* Tunisia */
	"xn--j1amh",		/* Ukraine */
	"xn--mgbaam7a8h",	/* United Arab Emirates */
#	if 0
	/* purposed IDN ccTLDs */
	"xn--54b7fta0cc",	/* Bangladesh */
	"xn--node",			/* Georgia */
	"xn--mgbai9azgqp6j",	/* Pakistan */
	"xn--mgbpl2fh",		/* Sudan */
	"xn--mgb2ddes",		/* Yemen */
#	endif /* 0 */
#	if 0
	/* Desired Variant String(s) IDN ccTLDs */
	"xn--mgba3a4fra",	/* Iran */
	"xn--mgbai9a5eva00b",	/* Pakistan */
	"xn--mgberp4a5d4a87g",	/* Saudi Arabia */
	"xn--mgbqly7c0a67fbc",	/* Saudi Arabia */
	"xn--mgbqly7cvafr",	/* Saudi Arabia */
	"xn--mgbtf8fl",		/* Syria */
	"xn--nnx388a",		/* Taiwan */
#	endif /* 0 */
	/* active IDN gTLDs */
	"xn--ngbc5azd",		/* Arabic for "web/network" */
	"xn--80asehdb",		/* Cyrillic for "online" */
	"xn--80aswg",		/* Cyrillic for "site" */
	"xn--unup4y",		/* Chinese for "game(s)" */
	/* puposed IDN gTLDs */
	/* sentinel */
	""
};
#endif /* !TIN_POLICY_H */
