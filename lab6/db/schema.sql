DROP TABLE IF EXISTS reviews;
DROP TABLE IF EXISTS bookings;
DROP TABLE IF EXISTS rooms;
DROP TABLE IF EXISTS hotels;
DROP TABLE IF EXISTS users;

CREATE TABLE users (
    id              BIGSERIAL PRIMARY KEY,
    first_name      VARCHAR(80) NOT NULL,
    last_name       VARCHAR(80) NOT NULL,
    login           VARCHAR(80) NOT NULL UNIQUE,
    email           VARCHAR(255) NOT NULL UNIQUE,
    password_hash   TEXT NOT NULL,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT users_email_format_check
        CHECK (position('@' in email) > 1),

    CONSTRAINT users_login_length_check
        CHECK (length(login) >= 3)
);

CREATE TABLE hotels (
    id          BIGSERIAL PRIMARY KEY,
    name        VARCHAR(160) NOT NULL,
    city        VARCHAR(120) NOT NULL,
    address     VARCHAR(255) NOT NULL,
    stars       SMALLINT NOT NULL,
    description TEXT,
    created_at  TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT hotels_stars_check
        CHECK (stars BETWEEN 1 AND 5),

    CONSTRAINT hotels_unique_name_address
        UNIQUE (name, address)
);

CREATE TABLE rooms (
    id              BIGSERIAL PRIMARY KEY,
    hotel_id        BIGINT NOT NULL REFERENCES hotels(id) ON DELETE CASCADE,
    room_number     VARCHAR(20) NOT NULL,
    room_type       VARCHAR(40) NOT NULL,
    capacity        SMALLINT NOT NULL,
    price_per_night NUMERIC(10, 2) NOT NULL,
    is_available    BOOLEAN NOT NULL DEFAULT TRUE,

    CONSTRAINT rooms_capacity_check
        CHECK (capacity > 0),

    CONSTRAINT rooms_price_check
        CHECK (price_per_night > 0),

    CONSTRAINT rooms_type_check
        CHECK (room_type IN ('standard', 'comfort', 'family', 'luxury')),

    CONSTRAINT rooms_unique_number_per_hotel
        UNIQUE (hotel_id, room_number)
);

CREATE TABLE bookings (
    id           BIGSERIAL PRIMARY KEY,
    user_id      BIGINT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    room_id      BIGINT NOT NULL REFERENCES rooms(id) ON DELETE RESTRICT,
    check_in     DATE NOT NULL,
    check_out    DATE NOT NULL,
    guests_count SMALLINT NOT NULL,
    status       VARCHAR(20) NOT NULL DEFAULT 'created',
    total_price  NUMERIC(10, 2) NOT NULL,
    created_at   TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT bookings_dates_check
        CHECK (check_out > check_in),

    CONSTRAINT bookings_guests_count_check
        CHECK (guests_count > 0),

    CONSTRAINT bookings_total_price_check
        CHECK (total_price >= 0),

    CONSTRAINT bookings_status_check
        CHECK (status IN ('created', 'confirmed', 'cancelled', 'completed'))
);

CREATE TABLE reviews (
    id          BIGSERIAL PRIMARY KEY,
    user_id     BIGINT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    hotel_id    BIGINT NOT NULL REFERENCES hotels(id) ON DELETE CASCADE,
    booking_id  BIGINT REFERENCES bookings(id) ON DELETE SET NULL,
    rating      SMALLINT NOT NULL,
    comment     TEXT,
    created_at  TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT reviews_rating_check
        CHECK (rating BETWEEN 1 AND 5),

    CONSTRAINT reviews_unique_user_hotel_booking
        UNIQUE (user_id, hotel_id, booking_id)
);


CREATE INDEX idx_rooms_hotel_id
    ON rooms (hotel_id);

CREATE INDEX idx_rooms_hotel_type_available
    ON rooms (hotel_id, room_type, is_available);

CREATE INDEX idx_hotels_city
    ON hotels (city);

CREATE INDEX idx_hotels_city_stars
    ON hotels (city, stars);

CREATE INDEX idx_bookings_user_id
    ON bookings (user_id);

CREATE INDEX idx_bookings_room_id
    ON bookings (room_id);

CREATE INDEX idx_bookings_room_dates_active
    ON bookings (room_id, check_in, check_out)
    WHERE status IN ('created', 'confirmed');

CREATE INDEX idx_bookings_status
    ON bookings (status);

CREATE INDEX idx_reviews_hotel_id
    ON reviews (hotel_id);

CREATE INDEX idx_reviews_user_id
    ON reviews (user_id);

CREATE INDEX idx_reviews_booking_id
    ON reviews (booking_id);
