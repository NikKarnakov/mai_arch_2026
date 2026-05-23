#include "storage.hpp"

#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/numeric_data.hpp>
#include <userver/storages/postgres/result_set.hpp>

namespace hotel {

namespace pg = userver::storages::postgres;

Storage::Storage(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      pg_(context.FindComponent<userver::components::Postgres>("hotel-booking-db").GetCluster()) {}

long Storage::CreateUser(const std::string& first_name,
                         const std::string& last_name,
                         const std::string& login,
                         const std::string& email,
                         const std::string& password_hash) const {
    const auto result = pg_->Execute(
        pg::ClusterHostType::kMaster,
        "INSERT INTO users(first_name, last_name, login, email, password_hash) "
        "VALUES($1, $2, $3, $4, $5) RETURNING id",
        first_name, last_name, login, email, password_hash);

    return result.AsSingleRow<long>();
}

std::optional<UserRow> Storage::GetUserByLogin(const std::string& login) const {
    const auto result = pg_->Execute(
        pg::ClusterHostType::kSlave,
        "SELECT id, first_name, last_name, login, email, password_hash FROM users WHERE login = $1",
        login);

    if (result.IsEmpty()) {
        return std::nullopt;
    }

    const auto row = result[0];
    return UserRow{
        row["id"].As<long>(),
        row["first_name"].As<std::string>(),
        row["last_name"].As<std::string>(),
        row["login"].As<std::string>(),
        row["email"].As<std::string>(),
        row["password_hash"].As<std::string>()
    };
}

std::vector<UserRow> Storage::SearchUsersByNameMask(const std::string& mask) const {
    const auto like_mask = "%" + mask + "%";
    const auto result = pg_->Execute(
        pg::ClusterHostType::kSlave,
        "SELECT id, first_name, last_name, login, email, password_hash "
        "FROM users "
        "WHERE first_name ILIKE $1 OR last_name ILIKE $1 "
        "ORDER BY last_name, first_name",
        like_mask);

    std::vector<UserRow> users;
    users.reserve(result.Size());

    for (const auto& row : result) {
        users.push_back(UserRow{
            row["id"].As<long>(),
            row["first_name"].As<std::string>(),
            row["last_name"].As<std::string>(),
            row["login"].As<std::string>(),
            row["email"].As<std::string>(),
            row["password_hash"].As<std::string>()
        });
    }

    return users;
}

long Storage::CreateHotel(const std::string& name,
                          const std::string& city,
                          const std::string& address,
                          int stars,
                          const std::string& description) const {
    const auto result = pg_->Execute(
        pg::ClusterHostType::kMaster,
        "INSERT INTO hotels(name, city, address, stars, description) "
        "VALUES($1, $2, $3, $4, $5) RETURNING id",
        name, city, address, stars, description);

    return result.AsSingleRow<long>();
}

std::vector<HotelRow> Storage::SearchHotels(const std::string& city, int min_stars) const {
    const auto result = pg_->Execute(
        pg::ClusterHostType::kSlave,
        "SELECT id, name, city, address, stars, COALESCE(description, '') AS description "
        "FROM hotels "
        "WHERE ($1 = '' OR city = $1) AND stars >= $2 "
        "ORDER BY stars DESC, name",
        city, min_stars);

    std::vector<HotelRow> hotels;
    hotels.reserve(result.Size());

    for (const auto& row : result) {
        hotels.push_back(HotelRow{
            row["id"].As<long>(),
            row["name"].As<std::string>(),
            row["city"].As<std::string>(),
            row["address"].As<std::string>(),
            row["stars"].As<int>(),
            row["description"].As<std::string>()
        });
    }

    return hotels;
}

std::vector<RoomRow> Storage::ListRooms(long hotel_id) const {
    const auto result = pg_->Execute(
        pg::ClusterHostType::kSlave,
        "SELECT id, hotel_id, room_number, room_type, capacity, "
        "price_per_night::double precision AS price_per_night, is_available "
        "FROM rooms WHERE hotel_id = $1 ORDER BY room_number",
        hotel_id);

    std::vector<RoomRow> rooms;
    rooms.reserve(result.Size());

    for (const auto& row : result) {
        rooms.push_back(RoomRow{
            row["id"].As<long>(),
            row["hotel_id"].As<long>(),
            row["room_number"].As<std::string>(),
            row["room_type"].As<std::string>(),
            row["capacity"].As<int>(),
            row["price_per_night"].As<double>(),
            row["is_available"].As<bool>()
        });
    }

    return rooms;
}

std::vector<RoomRow> Storage::FindAvailableRooms(long hotel_id,
                                                 const std::string& room_type,
                                                 int guests_count,
                                                 const std::string& check_in,
                                                 const std::string& check_out) const {
    const auto result = pg_->Execute(
        pg::ClusterHostType::kSlave,
        "SELECT r.id, r.hotel_id, r.room_number, r.room_type, r.capacity, "
        "       r.price_per_night::double precision AS price_per_night, r.is_available "
        "FROM rooms r "
        "WHERE r.hotel_id = $1 "
        "  AND ($2 = '' OR r.room_type = $2) "
        "  AND r.capacity >= $3 "
        "  AND r.is_available = TRUE "
        "  AND NOT EXISTS ( "
        "      SELECT 1 FROM bookings b "
        "      WHERE b.room_id = r.id "
        "        AND b.status IN ('created', 'confirmed') "
        "        AND b.check_in < $5::date "
        "        AND b.check_out > $4::date "
        "  ) "
        "ORDER BY r.price_per_night, r.room_number",
        hotel_id, room_type, guests_count, check_in, check_out);

    std::vector<RoomRow> rooms;
    rooms.reserve(result.Size());

    for (const auto& row : result) {
        rooms.push_back(RoomRow{
            row["id"].As<long>(),
            row["hotel_id"].As<long>(),
            row["room_number"].As<std::string>(),
            row["room_type"].As<std::string>(),
            row["capacity"].As<int>(),
            row["price_per_night"].As<double>(),
            row["is_available"].As<bool>()
        });
    }

    return rooms;
}

bool Storage::HasBookingConflict(long room_id,
                                 const std::string& check_in,
                                 const std::string& check_out) const {
    const auto result = pg_->Execute(
        pg::ClusterHostType::kSlave,
        "SELECT EXISTS ("
        "    SELECT 1 FROM bookings "
        "    WHERE room_id = $1 "
        "      AND status IN ('created', 'confirmed') "
        "      AND check_in < $3::date "
        "      AND check_out > $2::date"
        ")",
        room_id, check_in, check_out);

    return result.AsSingleRow<bool>();
}

long Storage::CreateBooking(long user_id,
                            long room_id,
                            const std::string& check_in,
                            const std::string& check_out,
                            int guests_count) const {
    auto tx = pg_->Begin("create_booking", pg::ClusterHostType::kMaster, {});

    const auto price_result = tx.Execute(
        "SELECT price_per_night::double precision "
        "FROM rooms WHERE id = $1 AND is_available = TRUE",
        room_id);

    if (price_result.IsEmpty()) {
        throw std::runtime_error("Room not found");
    }

    const auto conflict_result = tx.Execute(
        "SELECT EXISTS ("
        "    SELECT 1 FROM bookings "
        "    WHERE room_id = $1 "
        "      AND status IN ('created', 'confirmed') "
        "      AND check_in < $3::date "
        "      AND check_out > $2::date"
        ")",
        room_id, check_in, check_out);

    if (conflict_result.AsSingleRow<bool>()) {
        throw std::runtime_error("Room is already booked for selected dates");
    }

    const auto insert_result = tx.Execute(
        "INSERT INTO bookings(user_id, room_id, check_in, check_out, guests_count, status, total_price) "
        "SELECT $1, $2, $3::date, $4::date, $5, 'created', "
        "       price_per_night * (($4::date - $3::date)::numeric) "
        "FROM rooms WHERE id = $2 "
        "RETURNING id",
        user_id, room_id, check_in, check_out, guests_count);

    tx.Commit();
    return insert_result.AsSingleRow<long>();
}

std::vector<BookingRow> Storage::ListUserBookings(long user_id) const {
    const auto result = pg_->Execute(
        pg::ClusterHostType::kSlave,
        "SELECT id, user_id, room_id, check_in::text, check_out::text, guests_count, "
        "       status, total_price::double precision AS total_price "
        "FROM bookings WHERE user_id = $1 ORDER BY created_at DESC",
        user_id);

    std::vector<BookingRow> bookings;
    bookings.reserve(result.Size());

    for (const auto& row : result) {
        bookings.push_back(BookingRow{
            row["id"].As<long>(),
            row["user_id"].As<long>(),
            row["room_id"].As<long>(),
            row["check_in"].As<std::string>(),
            row["check_out"].As<std::string>(),
            row["guests_count"].As<int>(),
            row["status"].As<std::string>(),
            row["total_price"].As<double>()
        });
    }

    return bookings;
}

bool Storage::CancelBooking(long booking_id, long user_id) const {
    const auto result = pg_->Execute(
        pg::ClusterHostType::kMaster,
        "UPDATE bookings SET status = 'cancelled' "
        "WHERE id = $1 AND user_id = $2 AND status IN ('created', 'confirmed') "
        "RETURNING id",
        booking_id, user_id);

    return !result.IsEmpty();
}

long Storage::CreateReview(long user_id,
                           long hotel_id,
                           std::optional<long> booking_id,
                           int rating,
                           const std::string& comment) const {
    const auto result = pg_->Execute(
        pg::ClusterHostType::kMaster,
        "INSERT INTO reviews(user_id, hotel_id, booking_id, rating, comment) "
        "VALUES($1, $2, $3, $4, $5) RETURNING id",
        user_id, hotel_id, booking_id, rating, comment);

    return result.AsSingleRow<long>();
}

}  
