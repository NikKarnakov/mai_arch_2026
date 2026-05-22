DROP TABLE IF EXISTS bookings_partitioned CASCADE;

CREATE TABLE bookings_partitioned (
    id           BIGSERIAL,
    user_id      BIGINT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    room_id      BIGINT NOT NULL REFERENCES rooms(id) ON DELETE RESTRICT,
    check_in     DATE NOT NULL,
    check_out    DATE NOT NULL,
    guests_count SMALLINT NOT NULL,
    status       VARCHAR(20) NOT NULL DEFAULT 'created',
    total_price  NUMERIC(10, 2) NOT NULL,
    created_at   TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT bookings_partitioned_pk
        PRIMARY KEY (id, check_in),

    CONSTRAINT bookings_partitioned_dates_check
        CHECK (check_out > check_in),

    CONSTRAINT bookings_partitioned_guests_count_check
        CHECK (guests_count > 0),

    CONSTRAINT bookings_partitioned_total_price_check
        CHECK (total_price >= 0),

    CONSTRAINT bookings_partitioned_status_check
        CHECK (status IN ('created', 'confirmed', 'cancelled', 'completed'))
) PARTITION BY RANGE (check_in);

CREATE TABLE bookings_2026_q1
    PARTITION OF bookings_partitioned
    FOR VALUES FROM ('2026-01-01') TO ('2026-04-01');

CREATE TABLE bookings_2026_q2
    PARTITION OF bookings_partitioned
    FOR VALUES FROM ('2026-04-01') TO ('2026-07-01');

CREATE TABLE bookings_2026_q3
    PARTITION OF bookings_partitioned
    FOR VALUES FROM ('2026-07-01') TO ('2026-10-01');

CREATE TABLE bookings_2026_q4
    PARTITION OF bookings_partitioned
    FOR VALUES FROM ('2026-10-01') TO ('2027-01-01');

CREATE TABLE bookings_2027_q1
    PARTITION OF bookings_partitioned
    FOR VALUES FROM ('2027-01-01') TO ('2027-04-01');

CREATE INDEX idx_bookings_partitioned_user_id
    ON bookings_partitioned (user_id);

CREATE INDEX idx_bookings_partitioned_room_dates_active
    ON bookings_partitioned (room_id, check_in, check_out)
    WHERE status IN ('created', 'confirmed');

CREATE INDEX idx_bookings_partitioned_status
    ON bookings_partitioned (status);
