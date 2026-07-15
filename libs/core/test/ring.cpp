// /** @file `ring.cpp`
//  *  @brief
//  */

// #include "gbox/core/ring.hpp"

// #include <gtest/gtest.h>

// TEST(ringCreate, ringTests) {
//     const uint32_t ring_size = sizeof(float32_t);
//     const uint32_t ring_len = 4;

//     ring_t *ring = ring_new(ring_size, ring_len);
//     ASSERT_NE(nullptr, ring);

//     ASSERT_EQ(0, ring_destroy(&ring));
//     ASSERT_EQ(nullptr, ring);
// }

// TEST(ringDestroyNull, ringTests) {
//     const uint32_t ring_size = sizeof(float32_t);
//     const uint32_t ring_len = 4;

//     ASSERT_NE(0, ring_destroy(nullptr));
// }

// TEST(ringQueue, ringTests) {
//     const uint32_t ring_size = sizeof(float32_t);
//     const uint32_t ring_len = 4;
//     const uint32_t datum = 1;

//     ring_t *ring = ring_new(ring_size, ring_len);

//     ASSERT_NE(nullptr, ring);
//     ASSERT_EQ(0, ring_enqueue(ring, (void *)&datum));
//     ASSERT_EQ(0, ring_destroy(&ring));
// }

// TEST(ringQueueDequeueUint32, ringTests) {
//     const uint32_t ring_size = sizeof(int32_t);
//     const uint32_t ring_len = 2;

//     ring_t *ring = ring_new(ring_size, ring_len);

//     uint32_t datum = 10;
//     ASSERT_EQ(0, ring_enqueue(ring, (void *)&datum));

//     uint32_t old_datum = 0;
//     ASSERT_EQ(0, ring_dequeue(ring, (void *)&old_datum));

//     ASSERT_EQ(old_datum, datum);

//     ASSERT_EQ(0, ring_destroy(&ring));
// }

// TEST(ringQueueDequeue, ringTests) {
//     const uint32_t ring_size = sizeof(float32_t);
//     const uint32_t ring_len = 4;
//     ring_t *ring = ring_new(ring_size, ring_len);

//     ASSERT_NE(nullptr, ring);

//     const float32_t datum = 1.f;
//     float32_t new_datum = 0.f;

//     ASSERT_EQ(0, ring_enqueue(ring, (void *)&datum));
//     ASSERT_EQ(0, ring_dequeue(ring, (void *)&new_datum));

//     ASSERT_FLOAT_EQ(datum, new_datum);

//     ASSERT_EQ(0, ring_destroy(&ring));
//     ASSERT_EQ(nullptr, ring);
// }

// TEST(ringOverrun, ringTests) {
//     const uint32_t ring_size = sizeof(uint32_t);
//     const uint32_t ring_len = 2;
//     ring_t *ring = ring_new(ring_size, ring_len);

//     ASSERT_NE(nullptr, ring);

//     const uint32_t datum_replicant = 10.f;
//     ASSERT_EQ(0, ring_enqueue(ring, &datum_replicant));
//     ASSERT_EQ(0, ring_enqueue(ring, &datum_replicant));
//     ASSERT_NE(0, ring_enqueue(ring, &datum_replicant));

//     uint32_t datum_read = 0;
//     ASSERT_EQ(0, ring_dequeue(ring, &datum_read));
//     ASSERT_EQ(datum_read, datum_replicant);
//     ASSERT_EQ(0, ring_dequeue(ring, &datum_read));
//     ASSERT_EQ(datum_read, datum_replicant);

//     ASSERT_EQ(0, ring_destroy(&ring));
// }

// TEST(ringOverrunP1, ringTests) {
//     const uint32_t ring_size = sizeof(uint32_t);
//     const uint32_t ring_len = 2;
//     ring_t *ring = ring_new(ring_size, ring_len);

//     ASSERT_NE(nullptr, ring);

//     const uint32_t datum_replicant = 10.f;
//     ASSERT_EQ(0, ring_enqueue(ring, &datum_replicant));
//     ASSERT_EQ(0, ring_enqueue(ring, &datum_replicant));
//     ASSERT_NE(0, ring_enqueue(ring, &datum_replicant));

//     uint32_t datum_read = 0;
//     ASSERT_EQ(0, ring_dequeue(ring, &datum_read));
//     ASSERT_EQ(datum_read, datum_replicant);
//     ASSERT_EQ(0, ring_dequeue(ring, &datum_read));
//     ASSERT_EQ(datum_read, datum_replicant);
//     ASSERT_NE(0, ring_dequeue(ring, &datum_read));

//     ASSERT_EQ(0, ring_destroy(&ring));
// }
