db = db.getSiblingDB("hotel_booking_docs");

print("\n--- Aggregation: hotel revenue report ---");

db.bookings.aggregate([
  {
    $match: {
      status: { $in: ["created", "confirmed", "completed"] },
      "period.checkIn": {
        $gte: ISODate("2026-05-01T00:00:00Z"),
        $lt: ISODate("2026-10-01T00:00:00Z")
      }
    }
  },
  {
    $group: {
      _id: {
        hotelId: "$hotelId",
        hotelName: "$hotelSnapshot.name",
        city: "$hotelSnapshot.city",
        stars: "$hotelSnapshot.stars"
      },
      bookingsCount: { $sum: 1 },
      totalRevenue: { $sum: "$price.totalPrice" },
      averageBookingPrice: { $avg: "$price.totalPrice" },
      totalNights: { $sum: "$period.nights" },
      guestsCount: {
        $sum: {
          $add: ["$guests.adults", "$guests.children"]
        }
      },
      statuses: { $addToSet: "$status" }
    }
  },
  {
    $project: {
      _id: 0,
      hotelId: "$_id.hotelId",
      hotelName: "$_id.hotelName",
      city: "$_id.city",
      stars: "$_id.stars",
      bookingsCount: 1,
      totalRevenue: 1,
      averageBookingPrice: { $round: ["$averageBookingPrice", 2] },
      totalNights: 1,
      guestsCount: 1,
      statuses: 1,
      revenuePerNight: {
        $round: [
          {
            $cond: [
              { $eq: ["$totalNights", 0] },
              0,
              { $divide: ["$totalRevenue", "$totalNights"] }
            ]
          },
          2
        ]
      }
    }
  },
  {
    $sort: {
      totalRevenue: -1,
      bookingsCount: -1,
      hotelName: 1
    }
  }
]).forEach(printjson);


print("\n--- Aggregation: monthly booking status statistics ---");

db.bookings.aggregate([
  {
    $match: {
      "period.checkIn": {
        $gte: ISODate("2026-05-01T00:00:00Z"),
        $lt: ISODate("2026-10-01T00:00:00Z")
      }
    }
  },
  {
    $project: {
      month: {
        $dateToString: {
          format: "%Y-%m",
          date: "$period.checkIn"
        }
      },
      status: 1,
      totalPrice: "$price.totalPrice"
    }
  },
  {
    $group: {
      _id: {
        month: "$month",
        status: "$status"
      },
      count: { $sum: 1 },
      revenue: { $sum: "$totalPrice" }
    }
  },
  {
    $project: {
      _id: 0,
      month: "$_id.month",
      status: "$_id.status",
      count: 1,
      revenue: 1
    }
  },
  {
    $sort: {
      month: 1,
      status: 1
    }
  }
]).forEach(printjson);


print("\n--- Aggregation: hotel reviews rating report ---");

db.reviews.aggregate([
  {
    $match: {
      "moderation.status": "approved",
      rating: { $gte: 3 }
    }
  },
  {
    $group: {
      _id: "$hotelId",
      reviewsCount: { $sum: 1 },
      averageRating: { $avg: "$rating" },
      maxRating: { $max: "$rating" },
      minRating: { $min: "$rating" },
      travelerTypes: { $addToSet: "$travelerType" },
      commonPros: { $push: "$pros" }
    }
  },
  {
    $project: {
      _id: 0,
      hotelId: "$_id",
      reviewsCount: 1,
      averageRating: { $round: ["$averageRating", 2] },
      maxRating: 1,
      minRating: 1,
      travelerTypes: 1,
      commonPros: 1
    }
  },
  {
    $sort: {
      averageRating: -1,
      reviewsCount: -1
    }
  }
]).forEach(printjson);

print("\nAggregation queries finished");
