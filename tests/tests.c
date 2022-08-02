/*
 * This file is part of the flashrom project.
 *
 * Copyright 2020 Google LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <include/test.h>
#include "io_mock.h"
#include "tests.h"
#include "wraps.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

void *not_null(void)
{
	return (void *)NON_ZERO;
}

/* Workaround for https://github.com/clibs/cmocka/issues/17 */
char *__wrap_strdup(const char *s)
{
	size_t len = strlen(s) + 1;
	void *new = malloc(len);
	if (new == NULL)
		return NULL;
	return (char *)memcpy(new, s, len);
}

void __wrap_physunmap(void *virt_addr, size_t len)
{
	LOG_ME;
}

void *__wrap_physmap(const char *descr, uintptr_t phys_addr, size_t len)
{
	LOG_ME;
	return NULL;
}

struct pci_dev mock_pci_dev = {
	.device_id = NON_ZERO,
};

struct pci_dev *__wrap_pcidev_init(void *devs, int bar)
{
	LOG_ME;
	return &mock_pci_dev;
}

uintptr_t __wrap_pcidev_readbar(void *dev, int bar)
{
	LOG_ME;
	return NON_ZERO;
}

void __wrap_sio_write(uint16_t port, uint8_t reg, uint8_t data)
{
	LOG_ME;
}

uint8_t __wrap_sio_read(uint16_t port, uint8_t reg)
{
	LOG_ME;
	return (uint8_t)mock();
}

static int mock_open(const char *pathname, int flags)
{
	if (get_io() && get_io()->open)
		return get_io()->open(get_io()->state, pathname, flags);

	if (get_io() && get_io()->fallback_open_state) {
		struct io_mock_fallback_open_state *io_state;
		unsigned int open_state_flags;

		io_state = get_io()->fallback_open_state;
		assert_true(io_state->noc < MAX_MOCK_OPEN);
		assert_non_null(io_state->paths[io_state->noc]);
		assert_string_equal(pathname, io_state->paths[io_state->noc]);
		open_state_flags = io_state->flags[io_state->noc];
		assert_int_equal(flags & open_state_flags, open_state_flags);
		io_state->noc++; // proceed to the next path upon next call.
	}

	return MOCK_FD;
}

int __wrap_open(const char *pathname, int flags)
{
	LOG_ME;
	return mock_open(pathname, flags);
}

int __wrap_open64(const char *pathname, int flags)
{
	LOG_ME;
	return mock_open(pathname, flags);
}

int __wrap___open64_2(const char *pathname, int flags)
{
	LOG_ME;
	return mock_open(pathname, flags);
}

int __wrap_ioctl(int fd, unsigned long int request, ...)
{
	LOG_ME;
	if (get_io() && get_io()->ioctl) {
		va_list args;
		int out;
		va_start(args, request);
		out = get_io()->ioctl(get_io()->state, fd, request, args);
		va_end(args);
		return out;
	}
	return 0;
}

int __wrap_write(int fd, const void *buf, size_t sz)
{
	LOG_ME;
	if (get_io() && get_io()->write)
		return get_io()->write(get_io()->state, fd, buf, sz);
	return sz;
}

int __wrap_read(int fd, void *buf, size_t sz)
{
	LOG_ME;
	if (get_io() && get_io()->read)
		return get_io()->read(get_io()->state, fd, buf, sz);
	return sz;
}

FILE *__wrap_fopen(const char *pathname, const char *mode)
{
	LOG_ME;
	if (get_io() && get_io()->fopen)
		return get_io()->fopen(get_io()->state, pathname, mode);
	return not_null();
}

FILE *__wrap_fopen64(const char *pathname, const char *mode)
{
	LOG_ME;
	if (get_io() && get_io()->fopen)
		return get_io()->fopen(get_io()->state, pathname, mode);
	return not_null();
}

FILE *__wrap_fdopen(int fd, const char *mode)
{
	LOG_ME;
	return not_null();
}

int __wrap_stat(const char *path, void *buf)
{
	LOG_ME;
	return 0;
}

int __wrap_stat64(const char *path, void *buf)
{
	LOG_ME;
	return 0;
}

int __wrap___xstat(const char *path, void *buf)
{
	LOG_ME;
	return 0;
}

int __wrap___xstat64(const char *path, void *buf)
{
	LOG_ME;
	return 0;
}

int __wrap_fstat(int fd, void *buf)
{
	LOG_ME;
	return 0;
}

int __wrap_fstat64(int fd, void *buf)
{
	LOG_ME;
	return 0;
}

int __wrap___fxstat(int fd, void *buf)
{
	LOG_ME;
	return 0;
}

int __wrap___fxstat64(int fd, void *buf)
{
	LOG_ME;
	return 0;
}

char *__wrap_fgets(char *buf, int len, FILE *fp)
{
	LOG_ME;
	if (get_io() && get_io()->fgets)
		return get_io()->fgets(get_io()->state, buf, len, fp);
	return NULL;
}

char *__wrap___fgets_chk(char *buf, int len, FILE *fp)
{
	LOG_ME;
	if (get_io() && get_io()->fgets)
		return get_io()->fgets(get_io()->state, buf, len, fp);
	return NULL;
}

size_t __wrap_fread(void *ptr, size_t size, size_t nmemb, FILE *fp)
{
	LOG_ME;
	if (get_io() && get_io()->fread)
		return get_io()->fread(get_io()->state, ptr, size, nmemb, fp);
	return nmemb;
}

size_t __wrap_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *fp)
{
	LOG_ME;
	return nmemb;
}

int __wrap_fflush(FILE *fp)
{
	LOG_ME;
	return 0;
}

int __wrap_fileno(FILE *fp)
{
	LOG_ME;
	return NON_ZERO;
}

int __wrap_fsync(int fd)
{
	LOG_ME;
	return 0;
}

int __wrap_setvbuf(FILE *fp, char *buf, int type, size_t size)
{
	LOG_ME;
	return 0;
}

int __wrap_fprintf(FILE *fp, const char *fmt, ...)
{
	LOG_ME;
	if (get_io() && get_io()->fprintf) {
		va_list args;
		int out;
		va_start(args, fmt);
		out = get_io()->fprintf(get_io()->state, fp, fmt, args);
		va_end(args);
		return out;
	}
	return 0;
}

int __wrap___vfprintf_chk(FILE *fp, const char *fmt, va_list args)
{
	LOG_ME;
	if (get_io() && get_io()->fprintf)
		return get_io()->fprintf(get_io()->state, fp, fmt, args);
	return 0;
}

int __wrap_fclose(FILE *fp)
{
	LOG_ME;
	if (get_io() && get_io()->fclose)
		return get_io()->fclose(get_io()->state, fp);
	return 0;
}

int __wrap_feof(FILE *fp)
{
	/* LOG_ME; */
	return 0;
}

int __wrap_ferror(FILE *fp)
{
	/* LOG_ME; */
	return 0;
}
void __wrap_clearerr(FILE *fp)
{
	/* LOG_ME; */
	return;
}

int __wrap_rget_io_perms(void)
{
	LOG_ME;
	return 0;
}

void __wrap_OUTB(unsigned char value, unsigned short port)
{
	/* LOG_ME; */
	if (get_io() && get_io()->outb)
		get_io()->outb(get_io()->state, value, port);
}

unsigned char __wrap_INB(unsigned short port)
{
	/* LOG_ME; */
	if (get_io() && get_io()->inb)
		return get_io()->inb(get_io()->state, port);
	return 0;
}

void __wrap_OUTW(unsigned short value, unsigned short port)
{
	/* LOG_ME; */
	if (get_io() && get_io()->outw)
		get_io()->outw(get_io()->state, value, port);
}

unsigned short __wrap_INW(unsigned short port)
{
	/* LOG_ME; */
	if (get_io() && get_io()->inw)
		return get_io()->inw(get_io()->state, port);
	return 0;
}

void __wrap_OUTL(unsigned int value, unsigned short port)
{
	/* LOG_ME; */
	if (get_io() && get_io()->outl)
		get_io()->outl(get_io()->state, value, port);
}

unsigned int __wrap_INL(unsigned short port)
{
	/* LOG_ME; */
	if (get_io() && get_io()->inl)
		return get_io()->inl(get_io()->state, port);
	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;

	if (argc > 1)
		cmocka_set_test_filter(argv[1]);

	cmocka_set_message_output(CM_OUTPUT_STDOUT);

	const struct CMUnitTest helpers_tests[] = {
		cmocka_unit_test(address_to_bits_test_success),
		cmocka_unit_test(bitcount_test_success),
		cmocka_unit_test(minmax_test_success),
		cmocka_unit_test(strcat_realloc_test_success),
		cmocka_unit_test(tolower_string_test_success),
		cmocka_unit_test(reverse_byte_test_success),
		cmocka_unit_test(reverse_bytes_test_success),
	};
	ret |= cmocka_run_group_tests_name("helpers.c tests", helpers_tests, NULL, NULL);

	const struct CMUnitTest flashrom_tests[] = {
		cmocka_unit_test(flashbuses_to_text_test_success),
	};
	ret |= cmocka_run_group_tests_name("flashrom.c tests", flashrom_tests, NULL, NULL);

	const struct CMUnitTest spi25_tests[] = {
		cmocka_unit_test(spi_write_enable_test_success),
		cmocka_unit_test(spi_write_disable_test_success),
		cmocka_unit_test(spi_read_chunked_test_success),
		cmocka_unit_test(probe_spi_rdid_test_success),
		cmocka_unit_test(probe_spi_rdid4_test_success),
		cmocka_unit_test(probe_spi_rems_test_success),
		cmocka_unit_test(probe_spi_res1_test_success),
		cmocka_unit_test(probe_spi_res2_test_success),
		cmocka_unit_test(probe_spi_res3_test_success),
		cmocka_unit_test(probe_spi_at25f_test_success),
		cmocka_unit_test(probe_spi_st95_test_success), /* spi95.c */
	};
	ret |= cmocka_run_group_tests_name("spi25.c tests", spi25_tests, NULL, NULL);

	const struct CMUnitTest lifecycle_tests[] = {
		cmocka_unit_test(dummy_basic_lifecycle_test_success),
		cmocka_unit_test(dummy_probe_lifecycle_test_success),
		cmocka_unit_test(dummy_probe_variable_size_test_success),
		cmocka_unit_test(nicrealtek_basic_lifecycle_test_success),
		cmocka_unit_test(raiden_debug_basic_lifecycle_test_success),
		cmocka_unit_test(dediprog_basic_lifecycle_test_success),
		cmocka_unit_test(linux_mtd_probe_lifecycle_test_success),
		cmocka_unit_test(linux_spi_probe_lifecycle_test_success),
		cmocka_unit_test(realtek_mst_basic_lifecycle_test_success),
	};
	ret |= cmocka_run_group_tests_name("lifecycle.c tests", lifecycle_tests, NULL, NULL);

	const struct CMUnitTest layout_tests[] = {
		cmocka_unit_test(included_regions_dont_overlap_test_success),
		cmocka_unit_test(included_regions_overlap_test_success),
		cmocka_unit_test(region_not_included_overlap_test_success),
		cmocka_unit_test(layout_pass_sanity_checks_test_success),
		cmocka_unit_test(layout_region_invalid_address_test_success),
		cmocka_unit_test(layout_region_invalid_range_test_success),
	};
	ret |= cmocka_run_group_tests_name("layout.c tests", layout_tests, NULL, NULL);

	const struct CMUnitTest chip_tests[] = {
		cmocka_unit_test(erase_chip_test_success),
		cmocka_unit_test(erase_chip_with_dummyflasher_test_success),
		cmocka_unit_test(read_chip_test_success),
		cmocka_unit_test(read_chip_with_dummyflasher_test_success),
		cmocka_unit_test(write_chip_test_success),
		cmocka_unit_test(write_chip_with_dummyflasher_test_success),
		cmocka_unit_test(verify_chip_test_success),
		cmocka_unit_test(verify_chip_with_dummyflasher_test_success),
	};
	ret |= cmocka_run_group_tests_name("chip.c tests", chip_tests, NULL, NULL);

	const struct CMUnitTest chip_wp_tests[] = {
		cmocka_unit_test(invalid_wp_range_dummyflasher_test_success),
		cmocka_unit_test(set_wp_range_dummyflasher_test_success),
		cmocka_unit_test(switch_wp_mode_dummyflasher_test_success),
		cmocka_unit_test(wp_init_from_status_dummyflasher_test_success),
		cmocka_unit_test(full_chip_erase_with_wp_dummyflasher_test_success),
		cmocka_unit_test(partial_chip_erase_with_wp_dummyflasher_test_success),
	};
	ret |= cmocka_run_group_tests_name("chip_wp.c tests", chip_wp_tests, NULL, NULL);

	return ret;
}
