db = db.getSiblingDB("hotel_booking_docs");

print("\n--- CREATE operations ---");

db.bookings.insertOne({
  bookingId: "BK-2026-0101",
  userId: "USR-0001",
  hotelId: "HTL-0002",
  roomId: "ROOM-0003",
  status: "created",
  period: {
    checkIn: ISODate("2026-10-01T00:00:00Z"),
    checkOut: ISODate("2026-10-04T00:00:00Z"),
    nights: 3
  },
  guests: {
    adults: 2,
    children: 0,
    guestNames: ["Ivan Petrov", "Anna Petrova"]
  },
  hotelSnapshot: {
    name: "Moscow Garden",
    city: "Moscow",
    address: "Tverskaya Street 15",
    stars: 4
  },
  roomSnapshot: {
    roomNumber: "301",
    roomType: "comfort",
    capacity: 2,
    pricePerNight: 6400
  },
  price: {
    currency: "RUB",
    pricePerNight: 6400,
    totalPrice: 19200,
    discount: 0
  },
  payment: {
    status: "pending",
    method: "card"
  },
  events: [
    {
      type: "created",
      message: "Booking was created from queries.js",
      createdAt: new Date()
    }
  ],
  createdAt: new Date(),
  updatedAt: new Date()
});

db.reviews.insertOne({
  reviewId: "REV-0101",
  userId: "USR-0001",
  hotelId: "HTL-0002",
  bookingId: "BK-2026-0101",
  rating: 5,
  comment: "Good room and fast check-in.",
  pros: ["fast check-in", "location"],
  cons: [],
  travelerType: "couple",
  moderation: {
    status: "pending",
    checkedBy: null,
    checkedAt: null
  },
  createdAt: new Date()
});

db.user_preferences.insertOne({
  userId: "USR-0101",
  favoriteCities: ["Moscow"],
  preferredRoomTypes: ["comfort"],
  amenities: ["wifi", "breakfast"],
  priceRange: {
    min: 4000,
    max: 11000,
    currency: "RUB"
  },
  lastSearch: {
    city: "Moscow",
    checkIn: ISODate("2026-10-01T00:00:00Z"),
    checkOut: ISODate("2026-10-04T00:00:00Z"),
    guests: 2
  },
  updatedAt: new Date()
});

db.booking_events.insertOne({
  eventId: "EVT-0101",
  bookingId: "BK-2026-0101",
  userId: "USR-0001",
  type: "created",
  payload: {
    source: "queries.js",
    status: "created"
  },
  createdAt: new Date()
});

print("Create operations finished");


print("\n--- READ operations ---");

print("\n1. Booking by bookingId with $eq");
printjson(
  db.bookings.findOne({
    bookingId: { $eq: "BK-2026-0001" }
  })
);

print("\n2. Bookings with status not cancelled using $ne");
db.bookings.find(
  { status: { $ne: "cancelled" } },
  { _id: 0, bookingId: 1, status: 1, userId: 1 }
).forEach(printjson);

print("\n3. Bookings where check-in is inside June 2026 using $gt and $lt");
db.bookings.find(
  {
    "period.checkIn": {
      $gt: ISODate("2026-06-01T00:00:00Z"),
      $lt: ISODate("2026-07-01T00:00:00Z")
    }
  },
  { _id: 0, bookingId: 1, status: 1, "period.checkIn": 1, "period.checkOut": 1 }
).forEach(printjson);

print("\n4. Bookings in selected cities using $in");
db.bookings.find(
  {
    "hotelSnapshot.city": {
      $in: ["Moscow", "Saint Petersburg", "Sochi"]
    }
  },
  { _id: 0, bookingId: 1, "hotelSnapshot.city": 1, "hotelSnapshot.name": 1 }
).forEach(printjson);

print("\n5. Conflict check using $and");
db.bookings.find(
  {
    $and: [
      { roomId: "ROOM-0001" },
      { status: { $in: ["created", "confirmed"] } },
      { "period.checkIn": { $lt: ISODate("2026-06-03T00:00:00Z") } },
      { "period.checkOut": { $gt: ISODate("2026-06-01T00:00:00Z") } }
    ]
  },
  { _id: 0, bookingId: 1, roomId: 1, status: 1, period: 1 }
).forEach(printjson);

print("\n6. Bookings by selected user or hotel using $or");
db.bookings.find(
  {
    $or: [
      { userId: "USR-0001" },
      { hotelId: "HTL-0002" }
    ]
  },
  { _id: 0, bookingId: 1, userId: 1, hotelId: 1, status: 1 }
).forEach(printjson);

print("\n7. Reviews with approved moderation and rating greater than 4");
db.reviews.find(
  {
    "moderation.status": "approved",
    rating: { $gt: 4 }
  },
  { _id: 0, reviewId: 1, hotelId: 1, rating: 1, comment: 1 }
).forEach(printjson);

print("\n8. Users who prefer breakfast amenity");
db.user_preferences.find(
  {
    amenities: "breakfast"
  },
  { _id: 0, userId: 1, amenities: 1, favoriteCities: 1 }
).forEach(printjson);

print("\n9. Cancelled or completed events");
db.booking_events.find(
  {
    type: { $in: ["cancelled", "completed"] }
  },
  { _id: 0, eventId: 1, bookingId: 1, type: 1, payload: 1 }
).forEach(printjson);


print("\n--- UPDATE operations ---");

print("\n10. Confirm booking and update payment status");
printjson(
  db.bookings.updateOne(
    {
      bookingId: "BK-2026-0101",
      status: { $eq: "created" }
    },
    {
      $set: {
        status: "confirmed",
        "payment.status": "paid",
        updatedAt: new Date()
      },
      $push: {
        events: {
          type: "confirmed",
          message: "Booking was confirmed",
          createdAt: new Date()
        }
      }
    }
  )
);

print("\n11. Add one guest with $push");
printjson(
  db.bookings.updateOne(
    { bookingId: "BK-2026-0101" },
    {
      $push: {
        "guests.guestNames": "Sergey Petrov"
      },
      $inc: {
        "guests.adults": 1
      },
      $set: {
        updatedAt: new Date()
      }
    }
  )
);

print("\n12. Remove guest with $pull");
printjson(
  db.bookings.updateOne(
    { bookingId: "BK-2026-0101" },
    {
      $pull: {
        "guests.guestNames": "Sergey Petrov"
      },
      $inc: {
        "guests.adults": -1
      },
      $set: {
        updatedAt: new Date()
      }
    }
  )
);

print("\n13. Add unique amenity with $addToSet");
printjson(
  db.user_preferences.updateOne(
    { userId: "USR-0001" },
    {
      $addToSet: {
        amenities: "spa"
      },
      $set: {
        updatedAt: new Date()
      }
    }
  )
);

print("\n14. Remove favorite city with $pull");
printjson(
  db.user_preferences.updateOne(
    { userId: "USR-0001" },
    {
      $pull: {
        favoriteCities: "Moscow"
      },
      $set: {
        updatedAt: new Date()
      }
    }
  )
);

print("\n15. Approve review");
printjson(
  db.reviews.updateOne(
    {
      reviewId: "REV-0101",
      "moderation.status": { $ne: "approved" }
    },
    {
      $set: {
        "moderation.status": "approved",
        "moderation.checkedBy": "moderator-1",
        "moderation.checkedAt": new Date()
      }
    }
  )
);

print("\n16. Apply discount to active Moscow bookings");
printjson(
  db.bookings.updateMany(
    {
      $and: [
        { "hotelSnapshot.city": "Moscow" },
        { status: { $in: ["created", "confirmed"] } }
      ]
    },
    {
      $set: {
        "price.discount": 500,
        updatedAt: new Date()
      }
    }
  )
);

print("\n17. Upsert user preferences");
printjson(
  db.user_preferences.updateOne(
    { userId: "USR-0200" },
    {
      $set: {
        favoriteCities: ["Kazan"],
        preferredRoomTypes: ["standard"],
        amenities: ["wifi"],
        priceRange: { min: 3000, max: 8000, currency: "RUB" },
        lastSearch: {
          city: "Kazan",
          checkIn: ISODate("2026-11-01T00:00:00Z"),
          checkOut: ISODate("2026-11-03T00:00:00Z"),
          guests: 1
        },
        updatedAt: new Date()
      }
    },
    { upsert: true }
  )
);


print("\n--- DELETE operations ---");

print("\n18. Delete one technical event");
printjson(
  db.booking_events.deleteOne({
    eventId: "EVT-0101"
  })
);

print("\n19. Delete pending reviews older than selected date");
printjson(
  db.reviews.deleteMany({
    $and: [
      { "moderation.status": "pending" },
      { createdAt: { $lt: ISODate("2026-01-01T00:00:00Z") } }
    ]
  })
);

print("\n20. Soft delete booking by status update");
printjson(
  db.bookings.updateOne(
    {
      bookingId: "BK-2026-0101",
      status: { $ne: "cancelled" }
    },
    {
      $set: {
        status: "cancelled",
        "payment.status": "refunded",
        updatedAt: new Date()
      },
      $push: {
        events: {
          type: "cancelled",
          message: "Booking was cancelled by user",
          createdAt: new Date()
        }
      }
    }
  )
);

print("\n21. Delete test preferences");
printjson(
  db.user_preferences.deleteOne({
    userId: "USR-0200"
  })
);

print("\nCRUD queries finished");
