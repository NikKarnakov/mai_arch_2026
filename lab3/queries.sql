INSERT INTO users (
    first_name,
    last_name,
    login,
    email,
    password_hash
)
VALUES ($1, $2, $3, $4, $5)
RETURNING id, first_name, last_name, login, email, created_at;

SELECT id, login, password_hash
FROM users
WHERE login = $1;

INSERT INTO hotels (
    name,
    city,
    address,
    stars,
    description
)
VALUES ($1, $2, $3, $4, $5)
RETURNING id, name, city, address, stars, description, created_at;

SELECT id, name, city, address, stars, description, created_at
FROM hotels
WHERE id = $1;

SELECT id, name, city, address, stars, description
FROM hotels
WHERE city = $1
  AND stars >= $2
ORDER BY stars DESC, name;

SELECT id, name, city, address, stars, description
FROM hotels
ORDER BY city, name
LIMIT $1 OFFSET $2;

INSERT INTO rooms (
    hotel_id,
    room_number,
    room_type,
    capacity,
    price_per_night,
    is_available
)
VALUES ($1, $2, $3, $4, $5, COALESCE($6, TRUE))
RETURNING id, hotel_id, room_number, room_type, capacity, price_per_night, is_available;

SELECT id, hotel_id, room_number, room_type, capacity, price_per_night, is_available
FROM rooms
WHERE hotel_id = $1
ORDER BY room_number;

SELECT r.id,
       r.hotel_id,
       r.room_number,
       r.room_type,
       r.capacity,
       r.price_per_night
FROM rooms r
WHERE r.hotel_id = $1
  AND r.room_type = $2
  AND r.capacity >= $3
  AND r.is_available = TRUE
  AND NOT EXISTS (
      SELECT 1
      FROM bookings b
      WHERE b.room_id = r.id
        AND b.status IN ('created', 'confirmed')
        AND b.check_in < $5
        AND b.check_out > $4
  )
ORDER BY r.price_per_night, r.room_number;

SELECT EXISTS (
    SELECT 1
    FROM bookings
    WHERE room_id = $1
      AND status IN ('created', 'confirmed')
      AND check_in < $3
      AND check_out > $2
) AS has_conflict;

INSERT INTO bookings (
    user_id,
    room_id,
    check_in,
    check_out,
    guests_count,
    status,
    total_price
)
VALUES ($1, $2, $3, $4, $5, 'created', $6)
RETURNING id, user_id, room_id, check_in, check_out, guests_count, status, total_price, created_at;

SELECT b.id,
       b.status,
       b.check_in,
       b.check_out,
       b.guests_count,
       b.total_price,
       b.created_at,
       h.name AS hotel_name,
       h.city AS hotel_city,
       r.room_number,
       r.room_type
FROM bookings b
JOIN rooms r ON r.id = b.room_id
JOIN hotels h ON h.id = r.hotel_id
WHERE b.user_id = $1
ORDER BY b.created_at DESC;

UPDATE bookings
SET status = 'cancelled'
WHERE id = $1
  AND user_id = $2
  AND status IN ('created', 'confirmed')
RETURNING id, status;

INSERT INTO reviews (
    user_id,
    hotel_id,
    booking_id,
    rating,
    comment
)
VALUES ($1, $2, $3, $4, $5)
RETURNING id, user_id, hotel_id, booking_id, rating, comment, created_at;

SELECT r.id,
       r.rating,
       r.comment,
       r.created_at,
       u.first_name,
       u.last_name
FROM reviews r
JOIN users u ON u.id = r.user_id
WHERE r.hotel_id = $1
ORDER BY r.created_at DESC;

SELECT h.id,
       h.name,
       COUNT(r.id) AS reviews_count,
       ROUND(AVG(r.rating)::numeric, 2) AS average_rating
FROM hotels h
LEFT JOIN reviews r ON r.hotel_id = h.id
WHERE h.id = $1
GROUP BY h.id, h.name;
