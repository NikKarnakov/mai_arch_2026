db = db.getSiblingDB("hotel_booking_docs");

print("\n--- Applying startup $jsonSchema validation for bookings ---");

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



print("Startup validation configured");
