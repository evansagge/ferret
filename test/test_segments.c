#include "test.h"
#include "index.h"

#define T 1
#define F 0

/***************************************************************************
 *
 * SegmentInfo
 *
 ***************************************************************************/

#define Asi_equal(si1, si2)\
    tst_si_eql(__LINE__, tc, si2, si1->name, si1->doc_cnt, si1->store)

#define Asi_has_vals(si, name, doc_cnt, store)\
    tst_si_eql(__LINE__, tc, si, name, doc_cnt, store)

static int tst_si_eql(int line_num,
                      tst_case *tc,
                      SegmentInfo *si,
                      const char *name,
                      int doc_cnt,
                      Store *store)
{
    if (tst_str_equal(line_num, tc, name, si->name) &&
        tst_int_equal(line_num, tc, doc_cnt, si->doc_cnt) &&
        tst_ptr_equal(line_num, tc, store, si->store)) {
        return true;
    }
    else {
        return false;
    }

}

static void test_si(tst_case *tc, void *data)
{
    Store *store = (Store *)data;
    SegmentInfo *si = si_new(estrdup("_1"), 10, store);
    Asi_has_vals(si, "_1", 10, store);
    store = open_fs_store("./test/testdir/store");
    si->name[1] = '2';
    si->doc_cnt += 2;
    si->store = store;
    Asi_has_vals(si, "_2", 12, store);
    Assert(!si_uses_compound_file(si), "no _2.cfs so should be false");
    store->touch(store, "_2.cfs");
    Assert(si_uses_compound_file(si), "_2.cfs exists so should be true");
    Assert(!si_has_deletions(si), "no _2.del so should be false");
    store->touch(store, "_2.del");
    Assert(si_has_deletions(si), "_2.del exists so should be true");
    Assert(!si_has_separate_norms(si), "no _2.s* so should be false");
    store->touch(store, "_2.s12");
    Assert(si_has_separate_norms(si), "_2.s* exists so should be true");
    si_destroy(si);
    store_deref(store);
}

/***************************************************************************
 *
 * SegmentInfos
 *
 ***************************************************************************/


void test_sis_add_del(tst_case *tc, void *data)
{
    Store *store = (Store *)data;
    SegmentInfos *sis = sis_new();
    SegmentInfo *seg0 = sis_new_segment(sis, 123, store);
    SegmentInfo *seg1 = sis_new_segment(sis,  98, store);
    SegmentInfo *seg2 = sis_new_segment(sis,   3, store);
    SegmentInfo *seg3 = sis_new_segment(sis,  87, store);
    SegmentInfo *seg4 = sis_new_segment(sis,  12, store);

    Asi_has_vals(seg0, "_0", 123, store);
    Asi_has_vals(seg1, "_1",  98, store);
    Asi_has_vals(seg2, "_2",   3, store);
    Asi_has_vals(seg3, "_3",  87, store);
    Asi_has_vals(seg4, "_4",  12, store);

    Apequal(seg0, sis->segs[0]);
    Apequal(seg1, sis->segs[1]);
    Apequal(seg2, sis->segs[2]);
    Apequal(seg3, sis->segs[3]);
    Apequal(seg4, sis->segs[4]);

    sis_del_from_to(sis, 1, 4);
    Apequal(seg0, sis->segs[0]);
    Apequal(seg4, sis->segs[1]);
    sis_del_at(sis, 0);

    Apequal(seg4, sis->segs[0]);
    sis_destroy(sis);
}

void test_sis_rw(tst_case *tc, void *data)
{
    f_u64 version;
    Store *store = (Store *)data;
    SegmentInfos *sis = sis_new();
    SegmentInfo *seg0 = sis_new_segment(sis, 51, store);
    SegmentInfo *seg1 = sis_new_segment(sis, 213, store);
    SegmentInfo *seg2 = sis_new_segment(sis, 23, store);
    SegmentInfo *seg3 = si_new(estrdup("_3"), 9, store);
    SegmentInfos *sis2, *sis3;

    Aiequal(0, sis_read_current_version(store));
    Aiequal(3, sis->size);
    Apequal(seg0, sis->segs[0]);
    Apequal(seg2, sis->segs[2]);
    sis_write(sis, store);
    version = sis_read_current_version(store);
    Assert(store->exists(store, "segments"),
           "segments file should have been created");
    sis2 = sis_read(store);
    Aiequal(3, sis2->size);
    Asi_equal(seg0, sis2->segs[0]);
    Asi_equal(seg1, sis2->segs[1]);
    Asi_equal(seg2, sis2->segs[2]);

    sis_add_si(sis2, seg3);
    Aiequal(4, sis2->size);
    sis_write(sis2, store);
    Aiequal(version + 1, sis_read_current_version(store));
    sis3 = sis_read(store);
    Aiequal(version + 1, sis3->version);
    Aiequal(4, sis3->size);
    Asi_equal(seg0, sis3->segs[0]);
    Asi_equal(seg3, sis3->segs[3]);

    sis_destroy(sis);
    sis_destroy(sis2);
    sis_destroy(sis3);
}

tst_suite *ts_segments(tst_suite *suite)
{
    Store *store = open_ram_store();

    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_si, store);
    tst_run_test(suite, test_sis_add_del, store);
    tst_run_test(suite, test_sis_rw, store);

    store_deref(store);
    return suite;
}
