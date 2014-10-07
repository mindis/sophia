
/*
 * sophia database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <libsr.h>
#include <libsv.h>
#include <libsd.h>
#include <libst.h>
#include <sophia.h>

static void
addv(sdbuild *b, uint64_t lsn, uint8_t flags, int *key)
{
	svlocal l;
	l.lsn         = lsn;
	l.flags       = flags;
	l.key         = key;
	l.keysize     = sizeof(int);
	l.value       = NULL;
	l.valuesize   = 0;
	l.valueoffset = 0;
	sv lv;
	svinit(&lv, &sv_localif, &l, NULL);
	sd_buildadd(b, &lv);
}

static void
sditer_gt0(stc *cx srunused)
{
	sra a;
	sr_allocinit(&a, sr_allocstd, NULL);
	srcomparator cmp = { sr_cmpu32, NULL };
	sr r;
	sr_init(&r, &a, NULL, &cmp);
	sdbuild b;
	sd_buildinit(&b, &r);
	t( sd_buildbegin(&b, sizeof(int)) == 0);

	int key = 7;
	addv(&b, 3, SVSET, &key);
	key = 8;
	addv(&b, 4, SVSET, &key);
	key = 9;
	addv(&b, 5, SVSET, &key);
	sd_buildend(&b);
	sd_buildcommit(&b);
	sdindex index;
	sd_indexinit(&index);
	t( sd_indexbegin(&index, &a, 0) == 0 );
	sdid id;
	memset(&id, 0, sizeof(id));
	t( sd_indexcommit(&index, &a, &id) == 0 );

	srfile f;
	sr_fileinit(&f, &a);
	t( sr_filenew(&f, "./0000.db") == 0 );
	t( sd_buildwrite(&b, &index, &f) == 0 );
	srmap map;
	t( sr_mapfile(&map, &f, 1) == 0 );

	sriter it;
	sr_iterinit(&it, &sd_iter, &r);
	sr_iteropen(&it, &map, 1);
	t( sr_iterhas(&it) == 1 );

	sv *v = sr_iterof(&it);
	t( *(int*)svkey(v) == 7);
	sr_iternext(&it);
	v = sr_iterof(&it);
	t( *(int*)svkey(v) == 8);
	sr_iternext(&it);
	v = sr_iterof(&it);
	t( *(int*)svkey(v) == 9);
	sr_iternext(&it);
	t( sr_iterhas(&it) == 0 );

	sr_fileclose(&f);
	t( sr_mapunmap(&map) == 0 );
	t( sr_fileunlink("./0000.db") == 0 );

	sd_indexfree(&index, &a);
	sd_buildfree(&b);
}

static void
sditer_gt1(stc *cx srunused)
{
	sra a;
	sr_allocinit(&a, sr_allocstd, NULL);
	srcomparator cmp = { sr_cmpu32, NULL };
	sr r;
	sr_init(&r, &a, NULL, &cmp);
	sdbuild b;
	sd_buildinit(&b, &r);
	t( sd_buildbegin(&b, sizeof(int)) == 0);

	int key = 7;
	addv(&b, 3, SVSET, &key);
	key = 8;
	addv(&b, 4, SVSET, &key);
	key = 9;
	addv(&b, 5, SVSET, &key);
	sd_buildend(&b);
	sd_buildcommit(&b);

	t( sd_buildbegin(&b, sizeof(int)) == 0);
	key = 10;
	addv(&b, 6, SVSET, &key);
	key = 11;
	addv(&b, 7, SVSET, &key);
	key = 13;
	addv(&b, 8, SVSET, &key);
	sd_buildend(&b);
	sd_buildcommit(&b);

	t( sd_buildbegin(&b, sizeof(int)) == 0);
	key = 15;
	addv(&b, 9, SVSET, &key);
	key = 18;
	addv(&b, 10, SVSET, &key);
	key = 20;
	addv(&b, 11, SVSET, &key);
	sd_buildend(&b);
	sd_buildcommit(&b);

	sdindex index;
	sd_indexinit(&index);
	t( sd_indexbegin(&index, &a, 0) == 0 );
	sdid id;
	memset(&id, 0, sizeof(id));
	t( sd_indexcommit(&index, &a, &id) == 0 );
	srfile f;
	sr_fileinit(&f, &a);
	t( sr_filenew(&f, "./0000.db") == 0 );
	t( sd_buildwrite(&b, &index, &f) == 0 );
	srmap map;
	t( sr_mapfile(&map, &f, 1) == 0 );

	sriter it;
	sr_iterinit(&it, &sd_iter, &r);
	sr_iteropen(&it, &map, 1);
	t( sr_iterhas(&it) == 1 );

	/* page 0 */
	t( sr_iterhas(&it) != 0 );
	sv *v = sr_iterof(&it);
	t( *(int*)svkey(v) == 7);
	sr_iternext(&it);
	v = sr_iterof(&it);
	t( *(int*)svkey(v) == 8);
	sr_iternext(&it);
	v = sr_iterof(&it);
	t( *(int*)svkey(v) == 9);
	sr_iternext(&it);

	/* page 1 */
	v = sr_iterof(&it);
	t( *(int*)svkey(v) == 10);
	sr_iternext(&it);
	v = sr_iterof(&it);
	t( *(int*)svkey(v) == 11);
	sr_iternext(&it);
	v = sr_iterof(&it);
	t( *(int*)svkey(v) == 13);
	sr_iternext(&it);

	/* page 2 */
	v = sr_iterof(&it);
	t( *(int*)svkey(v) == 15);
	sr_iternext(&it);
	v = sr_iterof(&it);
	t( *(int*)svkey(v) == 18);
	sr_iternext(&it);
	v = sr_iterof(&it);
	t( *(int*)svkey(v) == 20);
	sr_iternext(&it);
	t( sr_iterhas(&it) == 0 );

	sr_fileclose(&f);
	t( sr_mapunmap(&map) == 0 );
	t( sr_fileunlink("./0000.db") == 0 );

	sd_indexfree(&index, &a);
	sd_buildfree(&b);
}

stgroup *sditer_group(void)
{
	stgroup *group = st_group("sditer");
	st_groupadd(group, st_test("gt0", sditer_gt0));
	st_groupadd(group, st_test("gt1", sditer_gt1));
	return group;
}
