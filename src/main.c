/*
 * main.c: entry point for sigutils
 * Creation date: Thu Oct 20 22:56:46 2016
 */

#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <sigutils/agc.h>
#include <sigutils/iir.h>
#include <sigutils/ncqo.h>
#include <sigutils/pll.h>
#include <sigutils/sampling.h>
#include <sigutils/sigutils.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <test_list.h>
#include <test_param.h>

SUPRIVATE su_test_entry_t test_list[] = {
    SU_TEST_ENTRY(su_test_ncqo),
    SU_TEST_ENTRY(su_test_butterworth_lpf),
    SU_TEST_ENTRY(su_test_agc_transient),
    SU_TEST_ENTRY(su_test_agc_steady_rising),
    SU_TEST_ENTRY(su_test_agc_steady_falling),
    SU_TEST_ENTRY(su_test_pll),
    SU_TEST_ENTRY(su_test_costas_lock),
    SU_TEST_ENTRY(su_test_costas_bpsk),
    SU_TEST_ENTRY(su_test_costas_qpsk),
    SU_TEST_ENTRY(su_test_costas_qpsk_noisy),
    SU_TEST_ENTRY(su_test_clock_recovery),
    SU_TEST_ENTRY(su_test_clock_recovery_noisy),
    SU_TEST_ENTRY(su_test_channel_detector_qpsk),
    SU_TEST_ENTRY(su_test_channel_detector_qpsk_noisy),
    SU_TEST_ENTRY(su_test_channel_detector_real_capture),
    SU_TEST_ENTRY(su_test_specttuner_two_tones),
    SU_TEST_ENTRY(su_test_mat_file_regular),
    SU_TEST_ENTRY(su_test_mat_file_streaming),
};

SUPRIVATE void
help(const char *argv0)
{
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s [options] [test_start [test_end]]\n\n", argv0);
  fprintf(
      stderr,
      "Runs all unit tests for the sigutils library. Please note that unless\n"
      "test_end is passed too, test_start will indicate an individual test\n"
      "to run. If neither test_start nor test_end is passed, all tests will\n"
      "be executed\n\n");
  fprintf(stderr, "Options:\n\n");
  fprintf(
      stderr,
      "     -d, --dump            Dump tests results as MATLAB files\n");
  fprintf(
      stderr,
      "     -w, --wav             Dump tests results as WAV files\n");
  fprintf(
      stderr,
      "     -R, --raw             Dump tests results as raw (I/Q) files\n");
  fprintf(
      stderr,
      "     -c, --count           Print number of tests and exit\n");
  fprintf(
      stderr,
      "     -s, --buffer-size=S   Sets the signal buffer size for unit\n");
  fprintf(
      stderr,
      "                           tests. Default is %d samples\n",
      SU_TEST_SIGNAL_BUFFER_SIZE);
  fprintf(
      stderr,
      "     -r, --sample-rate=r   For WAV files, set the sampling "
      "frequency.\n");
  fprintf(
      stderr,
      "                           Default is %d samples per second\n",
      SU_SIGBUF_SAMPLING_FREQUENCY_DEFAULT);
  fprintf(
      stderr,
      "     -l, --list            Provides a list of available unit tests\n");
  fprintf(
      stderr,
      "                           with their corresponding test ID and exit\n");
  fprintf(stderr, "     -v, --version         Print sigutils version\n");
  fprintf(stderr, "     -h, --help            This help\n\n");
  fprintf(stderr, "(c) 2020 Gonzalo J. Caracedo <BatchDrake@gmail.com>\n");
}

SUPRIVATE void
version(void)
{
  fprintf(stderr, "sigutils " SIGUTILS_VERSION_STRING "\n");
  fprintf(stderr, "pkgversion: %s\n\n", sigutils_pkgversion());

  fprintf(stderr, "Copyright © 2020 Gonzalo José Carracedo Carballal\n");
  fprintf(
      stderr,
      "License GPLv3+: GNU GPL version 3 or later "
      "<http://gnu.org/licenses/gpl.html>\n");
}

SUPRIVATE void
list(const char *argv0)
{
  unsigned int test_count = sizeof(test_list) / sizeof(test_list[0]);
  unsigned int i;

  for (i = 0; i < test_count; ++i) {
    printf("  %3u %s\n", i, test_list[i].name);
  }

  printf("%s: %u unit tests available\n", argv0, test_count);
}

SUPRIVATE struct option long_options[] = {
    {"dump", no_argument, NULL, 'd'},
    {"wav", no_argument, NULL, 'w'},
    {"raw", no_argument, NULL, 'R'},
    {"buffer-size", required_argument, NULL, 's'},
    {"sample-rate", required_argument, NULL, 'r'},
    {"help", no_argument, NULL, 'h'},
    {"count", no_argument, NULL, 'c'},
    {"list", no_argument, NULL, 'l'},
    {"version", no_argument, NULL, 'v'},
    {NULL, 0, NULL, 0}};

extern int optind;

int
main(int argc, char *argv[])
{
  unsigned int test_count = sizeof(test_list) / sizeof(test_list[0]);
  unsigned int test_start = 0;
  unsigned int test_end = test_count - 1;
  struct su_test_run_params params = su_test_run_params_INITIALIZER;
  SUBOOL result;
  int c;
  int index;

  while ((c = getopt_long(argc, argv, "Rdhclws:r:v", long_options, &index))
         != -1) {
    switch (c) {
      case 'c':
        printf("%s: %u unit tests available\n", argv[0], test_count);
        exit(EXIT_FAILURE);

      case 'd':
        params.dump_fmt = SU_DUMP_FORMAT_MATLAB;
        break;

      case 'R':
        params.dump_fmt = SU_DUMP_FORMAT_RAW;
        break;

      case 'w':
        params.dump_fmt = SU_DUMP_FORMAT_WAV;
        break;

      case 'h':
        help(argv[0]);
        exit(EXIT_SUCCESS);

      case 'l':
        list(argv[0]);
        exit(EXIT_SUCCESS);

      case 's':
        if (sscanf(optarg, "%" SCNu64, &params.buffer_size) < 0) {
          fprintf(stderr, "%s: invalid buffer size `%s'\n", argv[0], optarg);
          help(argv[0]);
          exit(EXIT_SUCCESS);
        }
        break;

      case 'r':
        if (sscanf(optarg, "%" SCNu64, &params.fs) < 0) {
          fprintf(stderr, "%s: invalid sampling rate `%s'\n", argv[0], optarg);
          help(argv[0]);
          exit(EXIT_SUCCESS);
        }
        break;

      case 'v':
        version();
        exit(EXIT_SUCCESS);
        break;

      case '?':
        help(argv[0]);
        exit(EXIT_FAILURE);

      default:
        /* Should never happen */
        abort();
    }
  }

  if (!su_lib_init()) {
    fprintf(stderr, "%s: failed to initialize sigutils library\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (argc - optind >= 1) {
    if (sscanf(argv[optind++], "%u", &test_start) < 1) {
      fprintf(
          stderr,
          "%s: invalid test start `%s'\n",
          argv[0],
          argv[optind - 1]);
      exit(EXIT_FAILURE);
    }

    test_end = test_start;
  }

  if (argc - optind >= 1) {
    if (sscanf(argv[optind++], "%u", &test_end) < 1) {
      fprintf(stderr, "%s: invalid test end\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (argc - optind >= 1) {
    fprintf(stderr, "%s: too many arguments\n", argv[0]);
    help(argv[0]);
    exit(EXIT_FAILURE);
  }

  result = su_test_run(
      test_list,
      test_count,
      SU_MIN(test_start, test_count - 1),
      SU_MIN(test_end, test_count - 1),
      &params);

  return !result;
}
