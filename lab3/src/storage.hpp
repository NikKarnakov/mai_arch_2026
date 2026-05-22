#pragma once

#include <optional>
#include <string>
#include <vector>

#include <userver/components/component_base.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

namespace hotel {

struct UserRow {
    long id{};
    std::string login;
    std::string password_hash;
};

struct HotelRow {
    long id{};
    std::string name;
    std::string city;
    std::string address;
    int stars{};
    std::string description;
};

struct RoomRow {
    long id{};
    long hotel_id{};
    std::string room_number;
    std::string room_type;
    int capacity{};
    double price_per_night{};
    bool is_available{};
};

struct BookingRow {
    long id{};
    long user_id{};
    long room_id{};
    std::string check_in;
    std::string check_out;
    int guests_count{};
    std::string status;
    double total_price{};
};

class Storage final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "storage";

    Storage(const userver::components::ComponentConfig& config,
            const userver::components::ComponentContext& context);

    long CreateUser(const std::string& first_name,
                    const std::string& last_name,
                    const std::string& login,
                    const std::string& email,
                    const std::string& password_hash) const;

    std::optional<UserRow> GetUserByLogin(const std::string& login) const;

    long CreateHotel(const std::string& name,
                     const std::string& city,
                     const std::string& address,
                     int stars,
                     const std::string& description) const;

    std::vector<HotelRow> SearchHotels(const std::string& city, int min_stars) const;

    std::vector<RoomRow> ListRooms(long hotel_id) const;

    std::vector<RoomRow> FindAvailableRooms(long hotel_id,
                                            const std::string& room_type,
                                            int guests_count,
                                            const std::string& check_in,
                                            const std::string& check_out) const;

    bool HasBookingConflict(long room_id,
                            const std::string& check_in,
                            const std::string& check_out) const;

    long CreateBooking(long user_id,
                       long room_id,
                       const std::string& check_in,
                       const std::string& check_out,
                       int guests_count) const;

    std::vector<BookingRow> ListUserBookings(long user_id) const;

    bool CancelBooking(long booking_id, long user_id) const;

    long CreateReview(long user_id,
                      long hotel_id,
                      std::optional<long> booking_id,
                      int rating,
                      const std::string& comment) const;

private:
    userver::storages::postgres::ClusterPtr pg_;
};

}
