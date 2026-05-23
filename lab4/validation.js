db = db.getSiblingDB("hotel_booking_docs");

print("\n--- Applying $jsonSchema validation for bookings ---");

db.runCommand({
  collMod: "bookings",
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: [
        "bookingId",
        "userId",
        "hotelId",
        "roomId",
        "status",
        "period",
        "guests",
        "hotelSnapshot",
        "roomSnapshot",
        "price",
        "payment",
        "events",
        "createdAt",
        "updatedAt"
      ],
      additionalProperties: true,
      properties: {
        bookingId: {
          bsonType: "string",
          pattern: "^BK-[0-9]{4}-[0-9]{4}$",
          description: "bookingId must match BK-YYYY-NNNN"
        },
        userId: {
          bsonType: "string",
          pattern: "^USR-[0-9]{4}$",
          description: "userId must match USR-NNNN"
        },
        hotelId: {
          bsonType: "string",
          pattern: "^HTL-[0-9]{4}$",
          description: "hotelId must match HTL-NNNN"
        },
        roomId: {
          bsonType: "string",
          pattern: "^ROOM-[0-9]{4}$",
          description: "roomId must match ROOM-NNNN"
        },
        status: {
          enum: ["created", "confirmed", "cancelled", "completed"],
          description: "status must be one of allowed booking statuses"
        },
        period: {
          bsonType: "object",
          required: ["checkIn", "checkOut", "nights"],
          properties: {
            checkIn: {
              bsonType: "date"
            },
            checkOut: {
              bsonType: "date"
            },
            nights: {
              bsonType: "int",
              minimum: 1,
              maximum: 60
            }
          }
        },
        guests: {
          bsonType: "object",
          required: ["adults", "children", "guestNames"],
          properties: {
            adults: {
              bsonType: "int",
              minimum: 1,
              maximum: 10
            },
            children: {
              bsonType: "int",
              minimum: 0,
              maximum: 10
            },
            guestNames: {
              bsonType: "array",
              minItems: 1,
              items: {
                bsonType: "string",
                minLength: 2
              }
            }
          }
        },
        hotelSnapshot: {
          bsonType: "object",
          required: ["name", "city", "address", "stars"],
          properties: {
            name: {
              bsonType: "string",
              minLength: 2,
              maxLength: 160
            },
            city: {
              bsonType: "string",
              minLength: 2,
              maxLength: 120
            },
            address: {
              bsonType: "string",
              minLength: 5,
              maxLength: 255
            },
            stars: {
              bsonType: "int",
              minimum: 1,
              maximum: 5
            }
          }
        },
        roomSnapshot: {
          bsonType: "object",
          required: ["roomNumber", "roomType", "capacity", "pricePerNight"],
          properties: {
            roomNumber: {
              bsonType: "string",
              minLength: 1,
              maxLength: 20
            },
            roomType: {
              enum: ["standard", "comfort", "family", "luxury"]
            },
            capacity: {
              bsonType: "int",
              minimum: 1,
              maximum: 10
            },
            pricePerNight: {
              bsonType: ["int", "double", "decimal"],
              minimum: 1
            }
          }
        },
        price: {
          bsonType: "object",
          required: ["currency", "pricePerNight", "totalPrice", "discount"],
          properties: {
            currency: {
              enum: ["RUB", "USD", "EUR"]
            },
            pricePerNight: {
              bsonType: ["int", "double", "decimal"],
              minimum: 1
            },
            totalPrice: {
              bsonType: ["int", "double", "decimal"],
              minimum: 1
            },
            discount: {
              bsonType: ["int", "double", "decimal"],
              minimum: 0
            }
          }
        },
        payment: {
          bsonType: "object",
          required: ["status", "method"],
          properties: {
            status: {
              enum: ["pending", "paid", "refunded", "failed"]
            },
            method: {
              enum: ["card", "sbp", "cash", "unknown"]
            }
          }
        },
        events: {
          bsonType: "array",
          minItems: 1,
          items: {
            bsonType: "object",
            required: ["type", "message", "createdAt"],
            properties: {
              type: {
                bsonType: "string",
                minLength: 2
              },
              message: {
                bsonType: "string",
                minLength: 2
              },
              createdAt: {
                bsonType: "date"
              }
            }
          }
        },
        createdAt: {
          bsonType: "date"
        },
        updatedAt: {
          bsonType: "date"
        }
      }
    }
  },
  validationLevel: "strict",
  validationAction: "error"
});

print("Validation has been applied to bookings collection");


print("\n--- Valid insert test ---");

try {
  db.bookings.insertOne({
    bookingId: "BK-2026-0901",
    userId: "USR-0001",
    hotelId: "HTL-0001",
    roomId: "ROOM-0001",
    status: "created",
    period: {
      checkIn: ISODate("2026-12-01T00:00:00Z"),
      checkOut: ISODate("2026-12-03T00:00:00Z"),
      nights: 2
    },
    guests: {
      adults: NumberInt(2),
      children: NumberInt(0),
      guestNames: ["Ivan Petrov", "Anna Petrova"]
    },
    hotelSnapshot: {
      name: "Nevsky Palace",
      city: "Saint Petersburg",
      address: "Nevsky Prospect 10",
      stars: NumberInt(5)
    },
    roomSnapshot: {
      roomNumber: "101",
      roomType: "standard",
      capacity: NumberInt(2),
      pricePerNight: 5200
    },
    price: {
      currency: "RUB",
      pricePerNight: 5200,
      totalPrice: 10400,
      discount: 0
    },
    payment: {
      status: "pending",
      method: "card"
    },
    events: [
      {
        type: "created",
        message: "Booking was created during validation test",
        createdAt: new Date()
      }
    ],
    createdAt: new Date(),
    updatedAt: new Date()
  });

  print("Valid document inserted successfully");
} catch (err) {
  print("Unexpected error for valid document:");
  printjson(err);
}


print("\n--- Invalid insert tests ---");

function expectValidationError(title, document) {
  print("\n" + title);

  try {
    db.bookings.insertOne(document);
    print("ERROR: invalid document was inserted, validation did not work");
  } catch (err) {
    print("OK: validation rejected invalid document");
    print("Error code: " + err.code);
    print("Error message: " + err.message);
  }
}

expectValidationError("1. Wrong bookingId pattern", {
  bookingId: "BAD-ID",
  userId: "USR-0001",
  hotelId: "HTL-0001",
  roomId: "ROOM-0001",
  status: "created",
  period: {
    checkIn: ISODate("2026-12-01T00:00:00Z"),
    checkOut: ISODate("2026-12-03T00:00:00Z"),
    nights: NumberInt(2)
  },
  guests: {
    adults: NumberInt(2),
    children: NumberInt(0),
    guestNames: ["Ivan Petrov"]
  },
  hotelSnapshot: {
    name: "Nevsky Palace",
    city: "Saint Petersburg",
    address: "Nevsky Prospect 10",
    stars: NumberInt(5)
  },
  roomSnapshot: {
    roomNumber: "101",
    roomType: "standard",
    capacity: NumberInt(2),
    pricePerNight: 5200
  },
  price: {
    currency: "RUB",
    pricePerNight: 5200,
    totalPrice: 10400,
    discount: 0
  },
  payment: {
    status: "pending",
    method: "card"
  },
  events: [
    {
      type: "created",
      message: "Invalid document",
      createdAt: new Date()
    }
  ],
  createdAt: new Date(),
  updatedAt: new Date()
});

expectValidationError("2. Unsupported booking status", {
  bookingId: "BK-2026-0902",
  userId: "USR-0001",
  hotelId: "HTL-0001",
  roomId: "ROOM-0001",
  status: "archived",
  period: {
    checkIn: ISODate("2026-12-01T00:00:00Z"),
    checkOut: ISODate("2026-12-03T00:00:00Z"),
    nights: NumberInt(2)
  },
  guests: {
    adults: NumberInt(2),
    children: NumberInt(0),
    guestNames: ["Ivan Petrov"]
  },
  hotelSnapshot: {
    name: "Nevsky Palace",
    city: "Saint Petersburg",
    address: "Nevsky Prospect 10",
    stars: NumberInt(5)
  },
  roomSnapshot: {
    roomNumber: "101",
    roomType: "standard",
    capacity: NumberInt(2),
    pricePerNight: 5200
  },
  price: {
    currency: "RUB",
    pricePerNight: 5200,
    totalPrice: 10400,
    discount: 0
  },
  payment: {
    status: "pending",
    method: "card"
  },
  events: [
    {
      type: "created",
      message: "Invalid document",
      createdAt: new Date()
    }
  ],
  createdAt: new Date(),
  updatedAt: new Date()
});

expectValidationError("3. Too many nights", {
  bookingId: "BK-2026-0903",
  userId: "USR-0001",
  hotelId: "HTL-0001",
  roomId: "ROOM-0001",
  status: "created",
  period: {
    checkIn: ISODate("2026-12-01T00:00:00Z"),
    checkOut: ISODate("2027-03-01T00:00:00Z"),
    nights: NumberInt(90)
  },
  guests: {
    adults: NumberInt(2),
    children: NumberInt(0),
    guestNames: ["Ivan Petrov"]
  },
  hotelSnapshot: {
    name: "Nevsky Palace",
    city: "Saint Petersburg",
    address: "Nevsky Prospect 10",
    stars: NumberInt(5)
  },
  roomSnapshot: {
    roomNumber: "101",
    roomType: "standard",
    capacity: NumberInt(2),
    pricePerNight: 5200
  },
  price: {
    currency: "RUB",
    pricePerNight: 5200,
    totalPrice: 468000,
    discount: 0
  },
  payment: {
    status: "pending",
    method: "card"
  },
  events: [
    {
      type: "created",
      message: "Invalid document",
      createdAt: new Date()
    }
  ],
  createdAt: new Date(),
  updatedAt: new Date()
});

expectValidationError("4. Missing required price object", {
  bookingId: "BK-2026-0904",
  userId: "USR-0001",
  hotelId: "HTL-0001",
  roomId: "ROOM-0001",
  status: "created",
  period: {
    checkIn: ISODate("2026-12-01T00:00:00Z"),
    checkOut: ISODate("2026-12-03T00:00:00Z"),
    nights: NumberInt(2)
  },
  guests: {
    adults: NumberInt(2),
    children: NumberInt(0),
    guestNames: ["Ivan Petrov"]
  },
  hotelSnapshot: {
    name: "Nevsky Palace",
    city: "Saint Petersburg",
    address: "Nevsky Prospect 10",
    stars: NumberInt(5)
  },
  roomSnapshot: {
    roomNumber: "101",
    roomType: "standard",
    capacity: NumberInt(2),
    pricePerNight: 5200
  },
  payment: {
    status: "pending",
    method: "card"
  },
  events: [
    {
      type: "created",
      message: "Invalid document",
      createdAt: new Date()
    }
  ],
  createdAt: new Date(),
  updatedAt: new Date()
});

expectValidationError("5. Hotel stars is out of range", {
  bookingId: "BK-2026-0905",
  userId: "USR-0001",
  hotelId: "HTL-0001",
  roomId: "ROOM-0001",
  status: "created",
  period: {
    checkIn: ISODate("2026-12-01T00:00:00Z"),
    checkOut: ISODate("2026-12-03T00:00:00Z"),
    nights: NumberInt(2)
  },
  guests: {
    adults: NumberInt(2),
    children: NumberInt(0),
    guestNames: ["Ivan Petrov"]
  },
  hotelSnapshot: {
    name: "Nevsky Palace",
    city: "Saint Petersburg",
    address: "Nevsky Prospect 10",
    stars: NumberInt(8)
  },
  roomSnapshot: {
    roomNumber: "101",
    roomType: "standard",
    capacity: NumberInt(2),
    pricePerNight: 5200
  },
  price: {
    currency: "RUB",
    pricePerNight: 5200,
    totalPrice: 10400,
    discount: 0
  },
  payment: {
    status: "pending",
    method: "card"
  },
  events: [
    {
      type: "created",
      message: "Invalid document",
      createdAt: new Date()
    }
  ],
  createdAt: new Date(),
  updatedAt: new Date()
});

print("\nValidation tests finished");
