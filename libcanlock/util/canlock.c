/* ========================================================================== */
/* Copyright (c) 2017 Michael Baeuerle
 *
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
 * OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY
 * SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * of the copyright holder.
 */

/* GNU autoconf */
#include <config.h>

/* C99 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Local */
#include "canlock.h"


/* ========================================================================== */
/* Constants */

/*! Size of buffer for secret data */
#define SEC_DATA_SIZE_MAX (size_t) 1024


/* ========================================================================== */
/*! \brief Print program version information */

static void print_version(void)
{
   const char*  p = PACKAGE_STRING;

   printf("%s\n", &p[3]);
   printf("%s\n", "Command line frontend for libcanlock.");

   return;
}


/* ========================================================================== */
/*! \brief Print help */

static void print_help(void)
{
   const char*  p = PACKAGE_NAME;

   printf("Usage: %s %s\n", &p[3], "[options]\n"          "Options:\n"
          "   -a scheme        Use <scheme> as hash algorithm for -k and -l "
                               "options\n"
          "                    (must be specified first)\n"
          "   -q               Suppress output to stdout for -c and -o "
                               "options\n"
          "                    (must be specified first)\n"
          "The following options are mutually exclusive:\n"
          "   -c c-key,c-lock  Check Cancel-Key <c-key> against "
                               "Cancel-Lock <c-lock>\n"
          "   -h               Print this help message to stdout and exit\n"
          "                    (must be the only option)\n"
          "   -k [uid]mid      Get Cancel-Key for Message-ID <mid>\n"
          "                    and an optional User-ID <uid>\n"
          "                    (secret is read from stdin)\n"
          "   -l [uid]mid      Get Cancel-Lock for Message-ID <mid>\n"
          "                    and an optional User-ID <uid>\n"
          "                    (secret is read from stdin)\n"
          "   -o               Print memory overwrite info to stdout and exit\n"
          "                    (must be the only option)\n"
          "   -v               Print version information to stdout and exit\n"
          "                    (must be the only option)\n"
          "(See manual page for details)"
   );

   return;
}


/* ========================================================================== */
/*! \brief Check scheme for supported hash algorithm
 *
 * \param[in] scheme  String with scheme in lowercase
 *
 * \return
 * - Hash algorithm ID on success
 * - \c CL_INVALID on error
 */

static cl_hash_version get_hash(const char *scheme)
{
   cl_hash_version hash = CL_INVALID;

   if (!strcmp(scheme, "sha1"))  { hash = CL_SHA1; }
   else if (!strcmp(scheme, "sha224"))  { hash = CL_SHA224; }
   else if (!strcmp(scheme, "sha256"))  { hash = CL_SHA256; }
   else if (!strcmp(scheme, "sha384"))  { hash = CL_SHA384; }
   else if (!strcmp(scheme, "sha512"))  { hash = CL_SHA512; }

   if (CL_INVALID == hash)
   {
      fprintf(stderr, "%s\n", "Hash algorithm not supported");
   }

   return (hash);
}


/* ========================================================================== */
/*! \brief Read secret data from stdin
 *
 * \param[out] sec_size  Pointer to size of secret data
 * \param[out] buf_size  Pointer to size of buffer
 *
 * Secret data is read until EOF.
 * Hitting the buffer size limit \c SEC_DATA_SIZE_MAX is treated as an error.
 *
 * \attention
 * The caller must not call \c free() for the returned pointer but should call
 * \ref cl_clear_secret() instead.
 *
 * \return
 * - Pointer to secret data on success
 * - \c NULL on error (nothing was written to \e secsize and \e bufsize )
 */

static unsigned char *get_secret(size_t *sec_size, size_t *buf_size)
{
   static unsigned char buf[SEC_DATA_SIZE_MAX];
   unsigned char *res = buf;
   int rv = 0;
   size_t i = 0;

   while (EOF != rv)
   {
      if (SEC_DATA_SIZE_MAX <= i)
      {
         res = NULL;
         break;
      }
      rv = fgetc(stdin);
      if (EOF != rv) { buf[i++] = (unsigned char) rv; }
   }
   cl_clear_secret((void *) &rv, sizeof(int), sizeof(int));
   if (NULL == res)
   {
      cl_clear_secret((void *) buf, SEC_DATA_SIZE_MAX, SEC_DATA_SIZE_MAX);
   }
   else
   {
      /* Return size of buffer and size of secret data */
      *buf_size = SEC_DATA_SIZE_MAX;
      *sec_size = i;
   }

   return (res);
}


/* ========================================================================== */
/*! \brief Execute request
 *
 * \param[in] hash       Hash algorithm ID
 * \param[in] quiet      Suppress result on stdout for check operation
 * \param[in] opt        Pointer to option string
 * \param[in] opt_value  Pointer to option value string
 *
 * \attention
 * The caller must ensure that \e opt and \e opt_val are not \c NULL .
 *
 * \return
 * - 0 on success
 * - -1 on error
 */

static int exec_request(cl_hash_version hash, int quiet,
                        const char *opt, const char *opt_value)
{
   int res = 0;
   enum { UK, KEY, LOCK, CHECK, OMEM } op = UK;
   unsigned char *sec = NULL;
   size_t  sec_size = 0, buf_size = 0;
   const unsigned char *mid;
   const char *out = NULL;
   char *key, *key_string, *lock, *lock_string;
   cl_hash_version key_hash, lock_hash;
   int memtest = 1;

   if (NULL == opt)  { return (-1); }
   if (strcmp(opt, "-o") && NULL == opt_value)  { return (-1); }

   if (!strcmp(opt, "-k")) { op = KEY; }
   if (!strcmp(opt, "-l")) { op = LOCK; }
   if (!strcmp(opt, "-c")) { op = CHECK; }
   if (!strcmp(opt, "-o")) { op = OMEM; }
   switch (op)
   {
      case KEY:
      case LOCK:
         /* Read secret data */
         sec = get_secret(&sec_size, &buf_size);
         if (NULL == sec)  { res = -1; }
         else
         {
            mid = (unsigned char *) opt_value;
            if (KEY == op)
            {
               out = cl_get_key(hash, sec, sec_size, mid, strlen(opt_value));
            }
            else
            {
               out = cl_get_lock(hash, sec, sec_size, mid, strlen(opt_value));
            }
            /* Remove secret data from memory */
            res = cl_clear_secret((void *) sec, sec_size, buf_size);
            if (0 > res)
            {
               fprintf(stderr, "%s\n", "Failed to overwrite secret data (bug)");
               res = -1;
            }
            else
            {
               if (0 < res)
               {
                  fprintf(stderr, "%s\n",
                          "Warning: Overwriting secret data was not reliable");
               }
               printf("%s\n", out);
               res = 0;
            }
            free((void *) out);
         }
         break;
      case CHECK:
         /* Split option value into key and lock part */
         key = (char *) malloc(strlen(opt_value) + (size_t) 1);
         if (NULL == key) { res = -1; }
         else
         {
            strcpy(key, opt_value);
            lock = strchr(key, (int) ',');
            if (NULL == lock)
            {
               fprintf(stderr, "%s\n", "Field separator not found (bug)");
               res = -1;
            }
            else { *lock++ = 0; }
            /* Check for colon after scheme ("-a" option is ignored) */
            if (NULL == strchr(key, (int) ':')
                || NULL == strchr(lock, (int) ':'))
            {
               fprintf(stderr, "%s\n", "Colon missing after scheme");
               res = -1;
            }
            else
            {
               /* Check whether key matches lock */
               key_hash = cl_split(key, &key_string);
               lock_hash = cl_split(lock, &lock_string);
               if (CL_INVALID == key_hash || CL_INVALID == lock_hash)
               {
                  fprintf(stderr, "%s\n", "Extracting scheme failed");
                  res = -1;
               }
               else
               {
                  if (key_hash != lock_hash)
                  {
                     /* Different scheme */
                     if(!quiet) { printf("%s\n", "Scheme mismatch"); }
                     res = -1;
                  }
                  else
                  {
                     if(cl_verify(key_hash, key_string, lock_string))
                     {
                        if(!quiet) { printf("%s\n", "Mismatch"); }
                        res = -1;
                     }
                     else
                     {
                        if(!quiet) { printf("%s\n", "Good"); }
                     }
                  }
               }
            }
            free((void *) key);
         }
         break;
      case OMEM:
         /* Check whether memory overwrite support is available */
         if (cl_clear_secret((void *) &memtest, sizeof(int), sizeof(int)))
         {
            if(!quiet)
            {
               printf("%s\n", "Memory overwrite support not available");
            }
            res = -1;
         }
         else
         {
            /* Available */
            if(!quiet) { printf("%s\n", "Memory overwrite support available"); }
         }
         break;
      case UK:
      default:
         fprintf(stderr, "%s\n", "Unknown operation (bug)");
         res = -1;
         break;
   }

   return (res);
}


/* ========================================================================== */
/*! \brief CLI utility
 *
 * \param[in] argc  Number of command line arguments
 * \param[in] argv  Array containing command line argument strings
 *
 * \return
 * - \c EXIT_SUCCESS on success
 * - \c EXIT_FAILURE on error
 */

int main(int argc, char **argv)
{
   int rv;
   enum { OK, ERROR, ABORT } rv2 = ERROR;
   unsigned int i;
   char option = 0;  /* Flag indicating option value will follow */
   cl_hash_version hash = CL_SHA256;  /* Mandatory algorithm as default */
   int quiet = 0;
   const char *opt = NULL;
   const char *opt_value = NULL;

   if (argc)
   {
      for (i = 1; i < (unsigned int) argc; i++)
      {
         if (option)
         {
            /* Process value of former option */
            opt_value = argv[i];
            if ('-' == opt_value[0])
            {
               fprintf(stderr, "%s\n", "Option value missing");
               break;
            }
            /* Accept option value */
            if (!strcmp(opt, "-a"))
            {
               hash = get_hash(opt_value);
               if (CL_INVALID == hash)
               {
                  break;
               }
               option = 0;
               /* Additional option must follow */
            }
            else
            {
               rv2 = OK;
               break;
            }
         }
         if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version"))
         {
            /* Print version information and report success */
            print_version();
            rv2 = ABORT;
            break;
         }
         else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
         {
            /* Print help and report success */
            print_help();
            rv2 = ABORT;
            break;
         }
         else if (!strcmp(argv[i], "-q"))
         {
            /* The "-c" or "-o" option should not print the result to stdout */
            quiet = 1;
            /* Additional option must follow */
         }
         else if (!strcmp(argv[i], "-o"))
         {
            opt = argv[i];
            rv2 = OK;
            break;
         }
         else if (!strcmp(argv[i], "-a")
                  || !strcmp(argv[i], "-c")
                  || !strcmp(argv[i], "-k")
                  || !strcmp(argv[i], "-l"))
         {
            /* Option value must follow */
            opt = argv[i];
            option = 1;
         }
         else if ('-' == argv[i][0])
         {
            /* Unknown option */
            fprintf(stderr, "%s\n", "Unknown option");
            break;
         }
      }
      if (ERROR == rv2)
      {
         fprintf(stderr, "%s\n", "Use '-h' or read the man page for help");
      }
   }

   /* Check options */
   switch (rv2)
   {
      case OK:
      {
         /* Check option value */
         if (strcmp(opt, "-o") && NULL == opt_value)
         {
            fprintf(stderr, "%s\n", "Option value missing (bug)");
            rv = EXIT_FAILURE;
            break;
         }
         else if (!strcmp(opt, "-c"))
         {
            if (NULL == strchr(opt_value, (int) ','))
            {
               fprintf(stderr, "%s\n", "Comma missing in option value");
               rv = EXIT_FAILURE;
               break;
            }
         }
         /* Execute requested operation */
         if (exec_request(hash, quiet, opt, opt_value)) { rv = EXIT_FAILURE; }
         else { rv = EXIT_SUCCESS; }
         break;
      }
      case ABORT:
      {
         /* Nothing more to do, but no error */
         rv = EXIT_SUCCESS;
         break;
      }
      case ERROR:
      default:
      {
         /* Error */
         rv = EXIT_FAILURE;
         break;
      }
   }

   exit(rv);
}


/* EOF */
