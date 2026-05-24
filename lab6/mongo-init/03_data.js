db = db.getSiblingDB("hotel_booking_docs");

db.bookings.deleteMany({});
db.reviews.deleteMany({});
db.user_preferences.deleteMany({});
db.booking_events.deleteMany({});

db.bookings.insertMany([
  {
    bookingId: "BK-2026-0001",
    userId: "USR-0001",
    hotelId: "HTL-0001",
    roomId: "ROOM-0001",
    status: "created",
    period: {
      checkIn: ISODate("2026-06-01T00:00:00Z"),
      checkOut: ISODate("2026-06-04T00:00:00Z"),
      nights: 3
    },
    guests: {
      adults: 2,
      children: 0,
      guestNames: ["Ivan Petrov", "Anna Petrova"]
    },
    hotelSnapshot: {
      name: "Nevsky Palace",
      city: "Saint Petersburg",
      address: "Nevsky Prospect 10",
      stars: 5
    },
    roomSnapshot: {
      roomNumber: "101",
      roomType: "standard",
      capacity: 2,
      pricePerNight: 5200
    },
    price: {
      currency: "RUB",
      pricePerNight: 5200,
      totalPrice: 15600,
      discount: 0
    },
    payment: {
      status: "pending",
      method: "card"
    },
    events: [
      {
        type: "created",
        message: "Booking was created",
        createdAt: ISODate("2026-05-22T10:00:00Z")
      }
    ],
    createdAt: ISODate("2026-05-22T10:00:00Z"),
    updatedAt: ISODate("2026-05-22T10:00:00Z")
  },
  {
    bookingId: "BK-2026-0002",
    userId: "USR-0002",
    hotelId: "HTL-0002",
    roomId: "ROOM-0003",
    status: "confirmed",
    period: { checkIn: ISODate("2026-06-03T00:00:00Z"), checkOut: ISODate("2026-06-05T00:00:00Z"), nights: 2 },
    guests: { adults: 2, children: 0, guestNames: ["Anna Sokolova", "Pavel Sokolov"] },
    hotelSnapshot: { name: "Moscow Garden", city: "Moscow", address: "Tverskaya Street 15", stars: 4 },
    roomSnapshot: { roomNumber: "301", roomType: "comfort", capacity: 2, pricePerNight: 6400 },
    price: { currency: "RUB", pricePerNight: 6400, totalPrice: 12800, discount: 0 },
    payment: { status: "paid", method: "card" },
    events: [
      { type: "created", message: "Booking was created", createdAt: ISODate("2026-05-23T09:00:00Z") },
      { type: "confirmed", message: "Payment confirmed", createdAt: ISODate("2026-05-23T09:05:00Z") }
    ],
    createdAt: ISODate("2026-05-23T09:00:00Z"),
    updatedAt: ISODate("2026-05-23T09:05:00Z")
  },
  {
    bookingId: "BK-2026-0003",
    userId: "USR-0003",
    hotelId: "HTL-0003",
    roomId: "ROOM-0005",
    status: "confirmed",
    period: { checkIn: ISODate("2026-06-10T00:00:00Z"), checkOut: ISODate("2026-06-12T00:00:00Z"), nights: 2 },
    guests: { adults: 1, children: 0, guestNames: ["Pavel Smirnov"] },
    hotelSnapshot: { name: "Baltic View", city: "Kaliningrad", address: "Leninsky Prospect 44", stars: 4 },
    roomSnapshot: { roomNumber: "11", roomType: "standard", capacity: 1, pricePerNight: 3900 },
    price: { currency: "RUB", pricePerNight: 3900, totalPrice: 7800, discount: 0 },
    payment: { status: "paid", method: "sbp" },
    events: [{ type: "created", message: "Booking was created", createdAt: ISODate("2026-05-24T13:20:00Z") }],
    createdAt: ISODate("2026-05-24T13:20:00Z"),
    updatedAt: ISODate("2026-05-24T13:20:00Z")
  },
  {
    bookingId: "BK-2026-0004",
    userId: "USR-0004",
    hotelId: "HTL-0005",
    roomId: "ROOM-0007",
    status: "confirmed",
    period: { checkIn: ISODate("2026-07-01T00:00:00Z"), checkOut: ISODate("2026-07-06T00:00:00Z"), nights: 5 },
    guests: { adults: 2, children: 1, guestNames: ["Maria Volkova", "Dmitry Volkov", "Kirill Volkov"] },
    hotelSnapshot: { name: "Sochi Sun", city: "Sochi", address: "Kurortny Prospect 100", stars: 5 },
    roomSnapshot: { roomNumber: "707", roomType: "luxury", capacity: 3, pricePerNight: 15600 },
    price: { currency: "RUB", pricePerNight: 15600, totalPrice: 78000, discount: 0 },
    payment: { status: "paid", method: "card" },
    events: [{ type: "created", message: "Booking was created", createdAt: ISODate("2026-05-25T08:40:00Z") }],
    createdAt: ISODate("2026-05-25T08:40:00Z"),
    updatedAt: ISODate("2026-05-25T08:40:00Z")
  },
  {
    bookingId: "BK-2026-0005",
    userId: "USR-0005",
    hotelId: "HTL-0006",
    roomId: "ROOM-0008",
    status: "cancelled",
    period: { checkIn: ISODate("2026-06-15T00:00:00Z"), checkOut: ISODate("2026-06-18T00:00:00Z"), nights: 3 },
    guests: { adults: 2, children: 0, guestNames: ["Dmitry Orlov", "Elena Orlova"] },
    hotelSnapshot: { name: "Ural Comfort", city: "Ekaterinburg", address: "Lenina Avenue 23", stars: 3 },
    roomSnapshot: { roomNumber: "15", roomType: "standard", capacity: 2, pricePerNight: 3100 },
    price: { currency: "RUB", pricePerNight: 3100, totalPrice: 9300, discount: 0 },
    payment: { status: "refunded", method: "card" },
    events: [
      { type: "created", message: "Booking was created", createdAt: ISODate("2026-05-25T12:00:00Z") },
      { type: "cancelled", message: "Booking was cancelled by user", createdAt: ISODate("2026-05-26T12:00:00Z") }
    ],
    createdAt: ISODate("2026-05-25T12:00:00Z"),
    updatedAt: ISODate("2026-05-26T12:00:00Z")
  },
  {
    bookingId: "BK-2026-0006",
    userId: "USR-0006",
    hotelId: "HTL-0008",
    roomId: "ROOM-0010",
    status: "created",
    period: { checkIn: ISODate("2026-08-02T00:00:00Z"), checkOut: ISODate("2026-08-08T00:00:00Z"), nights: 6 },
    guests: { adults: 2, children: 2, guestNames: ["Elena Kuznetsova", "Oleg Kuznetsov", "Mila Kuznetsova", "Nina Kuznetsova"] },
    hotelSnapshot: { name: "Volga Hotel", city: "Nizhny Novgorod", address: "Rozhdestvenskaya Street 12", stars: 3 },
    roomSnapshot: { roomNumber: "23", roomType: "family", capacity: 4, pricePerNight: 7600 },
    price: { currency: "RUB", pricePerNight: 7600, totalPrice: 45600, discount: 0 },
    payment: { status: "pending", method: "card" },
    events: [{ type: "created", message: "Booking was created", createdAt: ISODate("2026-05-27T10:10:00Z") }],
    createdAt: ISODate("2026-05-27T10:10:00Z"),
    updatedAt: ISODate("2026-05-27T10:10:00Z")
  },
  {
    bookingId: "BK-2026-0007",
    userId: "USR-0007",
    hotelId: "HTL-0009",
    roomId: "ROOM-0011",
    status: "confirmed",
    period: { checkIn: ISODate("2026-06-20T00:00:00Z"), checkOut: ISODate("2026-06-23T00:00:00Z"), nights: 3 },
    guests: { adults: 2, children: 0, guestNames: ["Nikita Morozov", "Olga Morozova"] },
    hotelSnapshot: { name: "Don Residence", city: "Rostov-on-Don", address: "Bolshaya Sadovaya 90", stars: 4 },
    roomSnapshot: { roomNumber: "404", roomType: "comfort", capacity: 2, pricePerNight: 5900 },
    price: { currency: "RUB", pricePerNight: 5900, totalPrice: 17700, discount: 0 },
    payment: { status: "paid", method: "card" },
    events: [{ type: "created", message: "Booking was created", createdAt: ISODate("2026-05-28T16:30:00Z") }],
    createdAt: ISODate("2026-05-28T16:30:00Z"),
    updatedAt: ISODate("2026-05-28T16:30:00Z")
  },
  {
    bookingId: "BK-2026-0008",
    userId: "USR-0008",
    hotelId: "HTL-0010",
    roomId: "ROOM-0012",
    status: "created",
    period: { checkIn: ISODate("2026-09-01T00:00:00Z"), checkOut: ISODate("2026-09-04T00:00:00Z"), nights: 3 },
    guests: { adults: 2, children: 0, guestNames: ["Olga Lebedeva", "Sergey Lebedev"] },
    hotelSnapshot: { name: "Primorye Harbor", city: "Vladivostok", address: "Svetlanskaya Street 32", stars: 4 },
    roomSnapshot: { roomNumber: "808", roomType: "luxury", capacity: 2, pricePerNight: 13200 },
    price: { currency: "RUB", pricePerNight: 13200, totalPrice: 39600, discount: 0 },
    payment: { status: "pending", method: "sbp" },
    events: [{ type: "created", message: "Booking was created", createdAt: ISODate("2026-05-29T09:45:00Z") }],
    createdAt: ISODate("2026-05-29T09:45:00Z"),
    updatedAt: ISODate("2026-05-29T09:45:00Z")
  },
  {
    bookingId: "BK-2026-0009",
    userId: "USR-0009",
    hotelId: "HTL-0002",
    roomId: "ROOM-0004",
    status: "completed",
    period: { checkIn: ISODate("2026-05-10T00:00:00Z"), checkOut: ISODate("2026-05-13T00:00:00Z"), nights: 3 },
    guests: { adults: 2, children: 2, guestNames: ["Sergey Fedorov", "Alina Fedorova", "Misha Fedorov", "Eva Fedorova"] },
    hotelSnapshot: { name: "Moscow Garden", city: "Moscow", address: "Tverskaya Street 15", stars: 4 },
    roomSnapshot: { roomNumber: "318", roomType: "family", capacity: 4, pricePerNight: 9700 },
    price: { currency: "RUB", pricePerNight: 9700, totalPrice: 29100, discount: 0 },
    payment: { status: "paid", method: "card" },
    events: [{ type: "completed", message: "Booking completed", createdAt: ISODate("2026-05-13T12:00:00Z") }],
    createdAt: ISODate("2026-05-01T11:15:00Z"),
    updatedAt: ISODate("2026-05-13T12:00:00Z")
  },
  {
    bookingId: "BK-2026-0010",
    userId: "USR-0010",
    hotelId: "HTL-0007",
    roomId: "ROOM-0009",
    status: "confirmed",
    period: { checkIn: ISODate("2026-06-25T00:00:00Z"), checkOut: ISODate("2026-06-27T00:00:00Z"), nights: 2 },
    guests: { adults: 2, children: 0, guestNames: ["Alina Mikhailova", "Maxim Mikhailov"] },
    hotelSnapshot: { name: "Siberia Plaza", city: "Novosibirsk", address: "Krasny Prospect 55", stars: 4 },
    roomSnapshot: { roomNumber: "1204", roomType: "comfort", capacity: 2, pricePerNight: 6100 },
    price: { currency: "RUB", pricePerNight: 6100, totalPrice: 12200, discount: 0 },
    payment: { status: "paid", method: "card" },
    events: [{ type: "created", message: "Booking was created", createdAt: ISODate("2026-05-30T14:00:00Z") }],
    createdAt: ISODate("2026-05-30T14:00:00Z"),
    updatedAt: ISODate("2026-05-30T14:00:00Z")
  }
]);

db.reviews.insertMany([
  {
    reviewId: "REV-0001",
    userId: "USR-0001",
    hotelId: "HTL-0001",
    bookingId: "BK-2026-0001",
    rating: 5,
    comment: "Clean room and very good location.",
    pros: ["location", "clean room", "friendly staff"],
    cons: ["small parking"],
    travelerType: "couple",
    moderation: { status: "approved", checkedBy: "system", checkedAt: ISODate("2026-06-05T10:00:00Z") },
    hotelReply: { message: "Thank you for your feedback!", createdAt: ISODate("2026-06-05T14:00:00Z") },
    createdAt: ISODate("2026-06-05T09:30:00Z")
  },
  {
    reviewId: "REV-0002",
    userId: "USR-0002",
    hotelId: "HTL-0002",
    bookingId: "BK-2026-0002",
    rating: 4,
    comment: "Good business hotel, breakfast was fine.",
    pros: ["breakfast", "metro nearby"],
    cons: ["small lobby"],
    travelerType: "business",
    moderation: { status: "approved", checkedBy: "system", checkedAt: ISODate("2026-06-06T10:00:00Z") },
    createdAt: ISODate("2026-06-06T09:00:00Z")
  },
  {
    reviewId: "REV-0003",
    userId: "USR-0003",
    hotelId: "HTL-0003",
    bookingId: "BK-2026-0003",
    rating: 4,
    comment: "Nice staff and quiet room.",
    pros: ["quiet", "staff"],
    cons: [],
    travelerType: "solo",
    moderation: { status: "approved", checkedBy: "system", checkedAt: ISODate("2026-06-13T08:20:00Z") },
    createdAt: ISODate("2026-06-13T08:00:00Z")
  },
  {
    reviewId: "REV-0004",
    userId: "USR-0004",
    hotelId: "HTL-0005",
    bookingId: "BK-2026-0004",
    rating: 5,
    comment: "Excellent sea view and service.",
    pros: ["sea view", "service", "pool"],
    cons: ["expensive restaurant"],
    travelerType: "family",
    moderation: { status: "approved", checkedBy: "moderator-1", checkedAt: ISODate("2026-07-07T12:00:00Z") },
    hotelReply: { message: "We are waiting for you again.", createdAt: ISODate("2026-07-07T16:30:00Z") },
    createdAt: ISODate("2026-07-07T11:30:00Z")
  },
  {
    reviewId: "REV-0005",
    userId: "USR-0005",
    hotelId: "HTL-0006",
    bookingId: "BK-2026-0005",
    rating: 3,
    comment: "Basic hotel, good for one night.",
    pros: ["price"],
    cons: ["old furniture"],
    travelerType: "couple",
    moderation: { status: "approved", checkedBy: "system", checkedAt: ISODate("2026-06-19T10:00:00Z") },
    createdAt: ISODate("2026-06-19T09:15:00Z")
  },
  {
    reviewId: "REV-0006",
    userId: "USR-0006",
    hotelId: "HTL-0008",
    bookingId: "BK-2026-0006",
    rating: 4,
    comment: "Large family room and friendly staff.",
    pros: ["large room", "staff"],
    cons: ["parking is paid"],
    travelerType: "family",
    moderation: { status: "pending", checkedBy: null, checkedAt: null },
    createdAt: ISODate("2026-08-09T12:10:00Z")
  },
  {
    reviewId: "REV-0007",
    userId: "USR-0007",
    hotelId: "HTL-0009",
    bookingId: "BK-2026-0007",
    rating: 4,
    comment: "Comfortable stay, good conference area.",
    pros: ["conference area", "clean"],
    cons: ["slow elevator"],
    travelerType: "business",
    moderation: { status: "approved", checkedBy: "system", checkedAt: ISODate("2026-06-24T15:00:00Z") },
    createdAt: ISODate("2026-06-24T14:40:00Z")
  },
  {
    reviewId: "REV-0008",
    userId: "USR-0008",
    hotelId: "HTL-0010",
    bookingId: "BK-2026-0008",
    rating: 5,
    comment: "Great view from the room.",
    pros: ["view", "location"],
    cons: [],
    travelerType: "couple",
    moderation: { status: "approved", checkedBy: "system", checkedAt: ISODate("2026-09-05T10:20:00Z") },
    createdAt: ISODate("2026-09-05T10:00:00Z")
  },
  {
    reviewId: "REV-0009",
    userId: "USR-0009",
    hotelId: "HTL-0002",
    bookingId: "BK-2026-0009",
    rating: 4,
    comment: "Convenient location in Moscow.",
    pros: ["location", "family room"],
    cons: ["busy street"],
    travelerType: "family",
    moderation: { status: "approved", checkedBy: "moderator-2", checkedAt: ISODate("2026-05-14T17:00:00Z") },
    createdAt: ISODate("2026-05-14T16:30:00Z")
  },
  {
    reviewId: "REV-0010",
    userId: "USR-0010",
    hotelId: "HTL-0007",
    bookingId: "BK-2026-0010",
    rating: 5,
    comment: "Modern hotel, everything was clean.",
    pros: ["modern", "clean"],
    cons: [],
    travelerType: "couple",
    moderation: { status: "approved", checkedBy: "system", checkedAt: ISODate("2026-06-28T09:00:00Z") },
    createdAt: ISODate("2026-06-28T08:30:00Z")
  }
]);

db.user_preferences.insertMany([
  {
    userId: "USR-0001",
    favoriteCities: ["Saint Petersburg", "Moscow"],
    preferredRoomTypes: ["standard", "comfort"],
    amenities: ["wifi", "parking", "breakfast"],
    priceRange: { min: 3000, max: 9000, currency: "RUB" },
    lastSearch: { city: "Moscow", checkIn: ISODate("2026-06-10T00:00:00Z"), checkOut: ISODate("2026-06-12T00:00:00Z"), guests: 2 },
    updatedAt: ISODate("2026-05-22T12:00:00Z")
  },
  {
    userId: "USR-0002",
    favoriteCities: ["Moscow", "Kazan"],
    preferredRoomTypes: ["comfort"],
    amenities: ["wifi", "gym"],
    priceRange: { min: 4500, max: 12000, currency: "RUB" },
    lastSearch: { city: "Kazan", checkIn: ISODate("2026-07-01T00:00:00Z"), checkOut: ISODate("2026-07-04T00:00:00Z"), guests: 2 },
    updatedAt: ISODate("2026-05-23T12:00:00Z")
  },
  {
    userId: "USR-0003",
    favoriteCities: ["Kaliningrad"],
    preferredRoomTypes: ["standard"],
    amenities: ["wifi"],
    priceRange: { min: 2500, max: 6000, currency: "RUB" },
    lastSearch: { city: "Kaliningrad", checkIn: ISODate("2026-06-10T00:00:00Z"), checkOut: ISODate("2026-06-12T00:00:00Z"), guests: 1 },
    updatedAt: ISODate("2026-05-24T12:00:00Z")
  },
  {
    userId: "USR-0004",
    favoriteCities: ["Sochi"],
    preferredRoomTypes: ["luxury", "family"],
    amenities: ["pool", "sea view", "spa"],
    priceRange: { min: 10000, max: 30000, currency: "RUB" },
    lastSearch: { city: "Sochi", checkIn: ISODate("2026-07-01T00:00:00Z"), checkOut: ISODate("2026-07-06T00:00:00Z"), guests: 3 },
    updatedAt: ISODate("2026-05-25T12:00:00Z")
  },
  {
    userId: "USR-0005",
    favoriteCities: ["Ekaterinburg"],
    preferredRoomTypes: ["standard"],
    amenities: ["parking"],
    priceRange: { min: 2000, max: 5000, currency: "RUB" },
    lastSearch: { city: "Ekaterinburg", checkIn: ISODate("2026-06-15T00:00:00Z"), checkOut: ISODate("2026-06-18T00:00:00Z"), guests: 2 },
    updatedAt: ISODate("2026-05-26T12:00:00Z")
  },
  {
    userId: "USR-0006",
    favoriteCities: ["Nizhny Novgorod", "Kazan"],
    preferredRoomTypes: ["family"],
    amenities: ["breakfast", "parking", "kids room"],
    priceRange: { min: 5000, max: 13000, currency: "RUB" },
    lastSearch: { city: "Nizhny Novgorod", checkIn: ISODate("2026-08-02T00:00:00Z"), checkOut: ISODate("2026-08-08T00:00:00Z"), guests: 4 },
    updatedAt: ISODate("2026-05-27T12:00:00Z")
  },
  {
    userId: "USR-0007",
    favoriteCities: ["Rostov-on-Don"],
    preferredRoomTypes: ["comfort"],
    amenities: ["conference room", "wifi"],
    priceRange: { min: 4000, max: 10000, currency: "RUB" },
    lastSearch: { city: "Rostov-on-Don", checkIn: ISODate("2026-06-20T00:00:00Z"), checkOut: ISODate("2026-06-23T00:00:00Z"), guests: 2 },
    updatedAt: ISODate("2026-05-28T12:00:00Z")
  },
  {
    userId: "USR-0008",
    favoriteCities: ["Vladivostok"],
    preferredRoomTypes: ["luxury"],
    amenities: ["sea view", "wifi", "breakfast"],
    priceRange: { min: 9000, max: 25000, currency: "RUB" },
    lastSearch: { city: "Vladivostok", checkIn: ISODate("2026-09-01T00:00:00Z"), checkOut: ISODate("2026-09-04T00:00:00Z"), guests: 2 },
    updatedAt: ISODate("2026-05-29T12:00:00Z")
  },
  {
    userId: "USR-0009",
    favoriteCities: ["Moscow", "Saint Petersburg"],
    preferredRoomTypes: ["family"],
    amenities: ["wifi", "breakfast", "metro nearby"],
    priceRange: { min: 7000, max: 16000, currency: "RUB" },
    lastSearch: { city: "Moscow", checkIn: ISODate("2026-05-10T00:00:00Z"), checkOut: ISODate("2026-05-13T00:00:00Z"), guests: 4 },
    updatedAt: ISODate("2026-05-01T12:00:00Z")
  },
  {
    userId: "USR-0010",
    favoriteCities: ["Novosibirsk"],
    preferredRoomTypes: ["comfort", "standard"],
    amenities: ["wifi", "gym", "parking"],
    priceRange: { min: 4000, max: 11000, currency: "RUB" },
    lastSearch: { city: "Novosibirsk", checkIn: ISODate("2026-06-25T00:00:00Z"), checkOut: ISODate("2026-06-27T00:00:00Z"), guests: 2 },
    updatedAt: ISODate("2026-05-30T12:00:00Z")
  }
]);

db.booking_events.insertMany([
  { eventId: "EVT-0001", bookingId: "BK-2026-0001", userId: "USR-0001", type: "created", payload: { source: "api", status: "created" }, createdAt: ISODate("2026-05-22T10:00:00Z") },
  { eventId: "EVT-0002", bookingId: "BK-2026-0002", userId: "USR-0002", type: "created", payload: { source: "api", status: "created" }, createdAt: ISODate("2026-05-23T09:00:00Z") },
  { eventId: "EVT-0003", bookingId: "BK-2026-0002", userId: "USR-0002", type: "status_changed", payload: { oldStatus: "created", newStatus: "confirmed", source: "payment-service" }, createdAt: ISODate("2026-05-23T09:05:00Z") },
  { eventId: "EVT-0004", bookingId: "BK-2026-0003", userId: "USR-0003", type: "created", payload: { source: "api", status: "confirmed" }, createdAt: ISODate("2026-05-24T13:20:00Z") },
  { eventId: "EVT-0005", bookingId: "BK-2026-0004", userId: "USR-0004", type: "created", payload: { source: "mobile", status: "confirmed" }, createdAt: ISODate("2026-05-25T08:40:00Z") },
  { eventId: "EVT-0006", bookingId: "BK-2026-0005", userId: "USR-0005", type: "created", payload: { source: "api", status: "created" }, createdAt: ISODate("2026-05-25T12:00:00Z") },
  { eventId: "EVT-0007", bookingId: "BK-2026-0005", userId: "USR-0005", type: "cancelled", payload: { reason: "plans changed", refund: true }, createdAt: ISODate("2026-05-26T12:00:00Z") },
  { eventId: "EVT-0008", bookingId: "BK-2026-0006", userId: "USR-0006", type: "created", payload: { source: "api", status: "created" }, createdAt: ISODate("2026-05-27T10:10:00Z") },
  { eventId: "EVT-0009", bookingId: "BK-2026-0007", userId: "USR-0007", type: "created", payload: { source: "partner", status: "confirmed" }, createdAt: ISODate("2026-05-28T16:30:00Z") },
  { eventId: "EVT-0010", bookingId: "BK-2026-0008", userId: "USR-0008", type: "created", payload: { source: "api", status: "created" }, createdAt: ISODate("2026-05-29T09:45:00Z") },
  { eventId: "EVT-0011", bookingId: "BK-2026-0009", userId: "USR-0009", type: "completed", payload: { auto: true, ratingRequested: true }, createdAt: ISODate("2026-05-13T12:00:00Z") },
  { eventId: "EVT-0012", bookingId: "BK-2026-0010", userId: "USR-0010", type: "created", payload: { source: "api", status: "confirmed" }, createdAt: ISODate("2026-05-30T14:00:00Z") }
]);

print("MongoDB test data loaded");
print("bookings:", db.bookings.countDocuments());
print("reviews:", db.reviews.countDocuments());
print("user_preferences:", db.user_preferences.countDocuments());
print("booking_events:", db.booking_events.countDocuments());
