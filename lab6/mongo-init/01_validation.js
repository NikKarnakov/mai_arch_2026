db = db.getSiblingDB("hotel_booking_docs");

db.createCollection("bookings");
db.createCollection("reviews");
db.createCollection("user_preferences");
db.createCollection("booking_events");

db.bookings.createIndex({ bookingId: 1 }, { unique: true });
db.bookings.createIndex({ userId: 1, createdAt: -1 });
db.bookings.createIndex({ hotelId: 1, "period.checkIn": 1 });
db.bookings.createIndex({ roomId: 1, "period.checkIn": 1, "period.checkOut": 1 });
db.bookings.createIndex({ status: 1 });

db.reviews.createIndex({ reviewId: 1 }, { unique: true });
db.reviews.createIndex({ hotelId: 1, createdAt: -1 });
db.reviews.createIndex({ userId: 1 });
db.reviews.createIndex({ bookingId: 1 }, { unique: true, sparse: true });

db.user_preferences.createIndex({ userId: 1 }, { unique: true });
db.user_preferences.createIndex({ favoriteCities: 1 });

db.booking_events.createIndex({ eventId: 1 }, { unique: true });
db.booking_events.createIndex({ bookingId: 1, createdAt: -1 });
db.booking_events.createIndex({ userId: 1, createdAt: -1 });
db.booking_events.createIndex({ type: 1 });
