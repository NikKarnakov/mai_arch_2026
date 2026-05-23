INSERT INTO users (first_name, last_name, login, email, password_hash) VALUES
('Ivan', 'Petrov', 'ivanp', 'ivan.petrov@example.com', '$demo$hash01'),
('Anna', 'Sokolova', 'annas', 'anna.sokolova@example.com', '$demo$hash02'),
('Pavel', 'Smirnov', 'pavels', 'pavel.smirnov@example.com', '$demo$hash03'),
('Maria', 'Volkova', 'mariav', 'maria.volkova@example.com', '$demo$hash04'),
('Dmitry', 'Orlov', 'dimao', 'dmitry.orlov@example.com', '$demo$hash05'),
('Elena', 'Kuznetsova', 'elenak', 'elena.kuznetsova@example.com', '$demo$hash06'),
('Nikita', 'Morozov', 'nikitam', 'nikita.morozov@example.com', '$demo$hash07'),
('Olga', 'Lebedeva', 'olgal', 'olga.lebedeva@example.com', '$demo$hash08'),
('Sergey', 'Fedorov', 'sergeyf', 'sergey.fedorov@example.com', '$demo$hash09'),
('Alina', 'Mikhailova', 'alinam', 'alina.mikhailova@example.com', '$demo$hash10');

INSERT INTO hotels (name, city, address, stars, description) VALUES
('Nevsky Palace', 'Saint Petersburg', 'Nevsky Prospect 10', 5, 'Отель недалеко от исторического центра города'),
('Moscow Garden', 'Moscow', 'Tverskaya Street 15', 4, 'Бизнес-отель недалеко от метро'),
('Baltic View', 'Kaliningrad', 'Leninsky Prospect 44', 4, 'Комфортабельный отель для поездок по городу'),
('Kazan Riverside', 'Kazan', 'Bauman Street 7', 4, 'Отель рядом с главной пешеходной улицей'),
('Sochi Sun', 'Sochi', 'Kurortny Prospect 100', 5, 'Курортный отель недалеко от моря'),
('Ural Comfort', 'Ekaterinburg', 'Lenina Avenue 23', 3, 'Простой отель для кратковременного проживания'),
('Siberia Plaza', 'Novosibirsk', 'Krasny Prospect 55', 4, 'Современный отель в центре города'),
('Volga Hotel', 'Nizhny Novgorod', 'Rozhdestvenskaya Street 12', 3, 'Гостиница рядом с набережной реки'),
('Don Residence', 'Rostov-on-Don', 'Bolshaya Sadovaya 90', 4, 'Отель с конференц-залами'),
('Primorye Harbor', 'Vladivostok', 'Svetlanskaya Street 32', 4, 'Отель с видом на море');

INSERT INTO rooms (hotel_id, room_number, room_type, capacity, price_per_night, is_available) VALUES
(1, '101', 'standard', 2, 5200.00, TRUE),
(1, '205', 'luxury', 2, 11800.00, TRUE),
(2, '301', 'comfort', 2, 6400.00, TRUE),
(2, '318', 'family', 4, 9700.00, TRUE),
(3, '11', 'standard', 1, 3900.00, TRUE),
(4, '42', 'comfort', 2, 5700.00, TRUE),
(5, '707', 'luxury', 3, 15600.00, TRUE),
(6, '15', 'standard', 2, 3100.00, TRUE),
(7, '1204', 'comfort', 2, 6100.00, TRUE),
(8, '23', 'family', 4, 7600.00, TRUE),
(9, '404', 'comfort', 2, 5900.00, TRUE),
(10, '808', 'luxury', 2, 13200.00, TRUE);

INSERT INTO bookings (user_id, room_id, check_in, check_out, guests_count, status, total_price) VALUES
(1, 1, '2026-06-01', '2026-06-04', 2, 'confirmed', 15600.00),
(2, 3, '2026-06-03', '2026-06-05', 2, 'created', 12800.00),
(3, 5, '2026-06-10', '2026-06-12', 1, 'confirmed', 7800.00),
(4, 7, '2026-07-01', '2026-07-06', 3, 'confirmed', 78000.00),
(5, 8, '2026-06-15', '2026-06-18', 2, 'cancelled', 9300.00),
(6, 10, '2026-08-02', '2026-08-08', 4, 'created', 45600.00),
(7, 11, '2026-06-20', '2026-06-23', 2, 'confirmed', 17700.00),
(8, 12, '2026-09-01', '2026-09-04', 2, 'created', 39600.00),
(9, 4, '2026-07-12', '2026-07-15', 4, 'completed', 29100.00),
(10, 9, '2026-06-25', '2026-06-27', 2, 'confirmed', 12200.00);

INSERT INTO reviews (user_id, hotel_id, booking_id, rating, comment) VALUES
(1, 1, 1, 5, 'Чистый номер и очень хорошее расположение.'),
(2, 2, 2, 4, 'Хороший бизнес-отель, завтрак был прекрасным.'),
(3, 3, 3, 4, 'Приятный персонал и тихая комната.'),
(4, 5, 4, 5, 'Отличный вид на море и сервис.'),
(5, 6, 5, 3, 'Обычный отель, подходящий для одной ночи.'),
(6, 8, 6, 4, 'Большой семейный номер и дружелюбный персонал.'),
(7, 9, 7, 4, 'Комфортное проживание, хорошая конференц-зона.'),
(8, 10, 8, 5, 'Прекрасный вид из окна.'),
(9, 2, 9, 4, 'Удобное расположение в Москве.'),
(10, 7, 10, 5, 'Современный отель, все было чисто.');
