-- TechZone Official Mall database (PostgreSQL)
-- Run in pgAdmin 4: right-click the 'techzone' database > Query Tool > paste > Execute (F5).

DROP TABLE IF EXISTS order_lines, orders, products, brands CASCADE;

CREATE TABLE brands (
    brand_id   SERIAL PRIMARY KEY,
    name       VARCHAR(30) NOT NULL UNIQUE,
    tagline    VARCHAR(120)
);

CREATE TABLE products (
    product_id INTEGER PRIMARY KEY,
    name       VARCHAR(80)  NOT NULL,
    category   VARCHAR(40)  NOT NULL,
    brand_id   INTEGER      NOT NULL REFERENCES brands(brand_id),
    price      NUMERIC(12,2) NOT NULL CHECK (price >= 0),
    stock      INTEGER      NOT NULL CHECK (stock >= 0),
    sold       INTEGER      NOT NULL DEFAULT 0,
    rating     NUMERIC(2,1) NOT NULL DEFAULT 0,
    discount   INTEGER      NOT NULL DEFAULT 0 CHECK (discount BETWEEN 0 AND 90)
);

CREATE TABLE orders (
    order_id   SERIAL PRIMARY KEY,
    customer   VARCHAR(40) NOT NULL DEFAULT 'You',
    status     VARCHAR(12) NOT NULL DEFAULT 'Pending'
               CHECK (status IN ('Pending','Completed','Failed')),
    total      NUMERIC(12,2) NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT NOW()
);

CREATE TABLE order_lines (
    line_id    SERIAL PRIMARY KEY,
    order_id   INTEGER NOT NULL REFERENCES orders(order_id) ON DELETE CASCADE,
    product_id INTEGER NOT NULL REFERENCES products(product_id),
    qty        INTEGER NOT NULL CHECK (qty > 0),
    unit_price NUMERIC(12,2) NOT NULL
);

CREATE INDEX idx_products_brand ON products(brand_id);
CREATE INDEX idx_products_price ON products(price);

INSERT INTO brands (name) VALUES
    ('Samsung'),
    ('Apple'),
    ('ASUS'),
    ('Lenovo'),
    ('Xiaomi'),
    ('OPPO'),
    ('Huawei'),
    ('HP'),
    ('Dell'),
    ('Acer');

INSERT INTO products (product_id, name, category, brand_id, price, stock, sold, rating, discount) VALUES
    (101, 'iPhone 11', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Apple'), 15990.00, 12, 177, 4.7, 5),
    (102, 'iPhone 12', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Apple'), 19990.00, 10, 214, 4.4, 10),
    (103, 'iPhone 13', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Apple'), 24990.00, 9, 251, 4.1, 15),
    (104, 'iPhone 14', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Apple'), 29990.00, 8, 288, 4.8, 20),
    (105, 'iPhone 15', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Apple'), 34990.00, 7, 325, 4.5, 0),
    (106, 'iPhone 16', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Apple'), 42990.00, 6, 362, 4.2, 5),
    (107, 'iPhone 17', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Apple'), 49990.00, 5, 399, 4.9, 10),
    (108, 'iPhone 17 Pro', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Apple'), 64990.00, 4, 436, 4.6, 15),
    (109, 'iPhone 17 Pro Max', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Apple'), 74990.00, 3, 473, 4.3, 20),
    (110, 'MacBook Air M3', 'Laptop', (SELECT brand_id FROM brands WHERE name='Apple'), 59990.00, 5, 510, 4.0, 0),
    (111, 'MacBook Pro M4', 'Laptop', (SELECT brand_id FROM brands WHERE name='Apple'), 99990.00, 2, 547, 4.7, 5),
    (201, 'Galaxy S23', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Samsung'), 24990.00, 10, 277, 4.7, 5),
    (202, 'Galaxy S24', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Samsung'), 32990.00, 9, 314, 4.4, 10),
    (203, 'Galaxy S25', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Samsung'), 39990.00, 7, 351, 4.1, 15),
    (204, 'Galaxy S26', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Samsung'), 45990.00, 5, 388, 4.8, 20),
    (205, 'Galaxy A15', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Samsung'), 7990.00, 18, 425, 4.5, 0),
    (206, 'Galaxy A35', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Samsung'), 16990.00, 12, 462, 4.2, 5),
    (207, 'Galaxy Tab S10', 'Tablet', (SELECT brand_id FROM brands WHERE name='Samsung'), 32990.00, 6, 499, 4.9, 10),
    (301, 'Vivobook 15', 'Laptop', (SELECT brand_id FROM brands WHERE name='ASUS'), 28990.00, 10, 377, 4.7, 5),
    (302, 'Vivobook 16', 'Laptop', (SELECT brand_id FROM brands WHERE name='ASUS'), 32990.00, 8, 414, 4.4, 10),
    (303, 'Zenbook 14', 'Laptop', (SELECT brand_id FROM brands WHERE name='ASUS'), 54990.00, 4, 451, 4.1, 15),
    (304, 'ROG Zephyrus G14', 'Gaming Laptop', (SELECT brand_id FROM brands WHERE name='ASUS'), 89990.00, 3, 488, 4.8, 20),
    (305, 'ROG Strix G16', 'Gaming Laptop', (SELECT brand_id FROM brands WHERE name='ASUS'), 79990.00, 4, 525, 4.5, 0),
    (401, 'IdeaPad Slim 3', 'Laptop', (SELECT brand_id FROM brands WHERE name='Lenovo'), 24990.00, 11, 477, 4.7, 5),
    (402, 'IdeaPad Slim 5', 'Laptop', (SELECT brand_id FROM brands WHERE name='Lenovo'), 34990.00, 7, 514, 4.4, 10),
    (403, 'ThinkPad E14', 'Business Laptop', (SELECT brand_id FROM brands WHERE name='Lenovo'), 39990.00, 6, 551, 4.1, 15),
    (404, 'ThinkPad T14', 'Business Laptop', (SELECT brand_id FROM brands WHERE name='Lenovo'), 64990.00, 3, 588, 4.8, 20),
    (405, 'Legion 5', 'Gaming Laptop', (SELECT brand_id FROM brands WHERE name='Lenovo'), 69990.00, 4, 625, 4.5, 0),
    (501, 'Redmi Note 13', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Xiaomi'), 11990.00, 15, 577, 4.7, 5),
    (502, 'Redmi Note 14', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Xiaomi'), 13990.00, 14, 614, 4.4, 10),
    (503, 'Xiaomi 14T', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Xiaomi'), 29990.00, 8, 651, 4.1, 15),
    (504, 'Xiaomi 15', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Xiaomi'), 44990.00, 6, 688, 4.8, 20),
    (505, 'Xiaomi Pad 6', 'Tablet', (SELECT brand_id FROM brands WHERE name='Xiaomi'), 17990.00, 7, 725, 4.5, 0),
    (601, 'OPPO A58', 'Smartphone', (SELECT brand_id FROM brands WHERE name='OPPO'), 8990.00, 16, 677, 4.7, 5),
    (602, 'OPPO Reno 11', 'Smartphone', (SELECT brand_id FROM brands WHERE name='OPPO'), 21990.00, 10, 714, 4.4, 10),
    (603, 'OPPO Reno 12', 'Smartphone', (SELECT brand_id FROM brands WHERE name='OPPO'), 24990.00, 9, 751, 4.1, 15),
    (604, 'OPPO Find X7', 'Smartphone', (SELECT brand_id FROM brands WHERE name='OPPO'), 49990.00, 4, 788, 4.8, 20),
    (605, 'OPPO Find N3 Flip', 'Foldable Phone', (SELECT brand_id FROM brands WHERE name='OPPO'), 54990.00, 3, 825, 4.5, 0),
    (701, 'Huawei Nova 12', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Huawei'), 17990.00, 9, 777, 4.7, 5),
    (702, 'Huawei Pura 70', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Huawei'), 44990.00, 5, 814, 4.4, 10),
    (703, 'Huawei Mate 60 Pro', 'Smartphone', (SELECT brand_id FROM brands WHERE name='Huawei'), 59990.00, 4, 851, 4.1, 15),
    (704, 'MateBook D 16', 'Laptop', (SELECT brand_id FROM brands WHERE name='Huawei'), 42990.00, 6, 888, 4.8, 20),
    (705, 'MatePad Pro 13', 'Tablet', (SELECT brand_id FROM brands WHERE name='Huawei'), 39990.00, 5, 925, 4.5, 0),
    (801, 'HP Pavilion 15', 'Laptop', (SELECT brand_id FROM brands WHERE name='HP'), 31990.00, 10, 877, 4.7, 5),
    (802, 'HP Pavilion Plus 14', 'Laptop', (SELECT brand_id FROM brands WHERE name='HP'), 45990.00, 7, 914, 4.4, 10),
    (803, 'HP Envy x360', 'Laptop', (SELECT brand_id FROM brands WHERE name='HP'), 52990.00, 5, 51, 4.1, 15),
    (804, 'HP Spectre x360', 'Premium Laptop', (SELECT brand_id FROM brands WHERE name='HP'), 89990.00, 3, 88, 4.8, 20),
    (805, 'HP Omen 16', 'Gaming Laptop', (SELECT brand_id FROM brands WHERE name='HP'), 74990.00, 4, 125, 4.5, 0),
    (901, 'Dell Inspiron 15', 'Laptop', (SELECT brand_id FROM brands WHERE name='Dell'), 29990.00, 11, 77, 4.7, 5),
    (902, 'Dell Inspiron 16', 'Laptop', (SELECT brand_id FROM brands WHERE name='Dell'), 36990.00, 8, 114, 4.4, 10),
    (903, 'Dell XPS 13', 'Premium Laptop', (SELECT brand_id FROM brands WHERE name='Dell'), 79990.00, 4, 151, 4.1, 15),
    (904, 'Dell XPS 15', 'Premium Laptop', (SELECT brand_id FROM brands WHERE name='Dell'), 99990.00, 3, 188, 4.8, 20),
    (905, 'Dell Alienware m16', 'Gaming Laptop', (SELECT brand_id FROM brands WHERE name='Dell'), 109990.00, 2, 225, 4.5, 0),
    (1001, 'Acer Aspire 3', 'Laptop', (SELECT brand_id FROM brands WHERE name='Acer'), 22990.00, 12, 177, 4.7, 5),
    (1002, 'Acer Aspire 5', 'Laptop', (SELECT brand_id FROM brands WHERE name='Acer'), 29990.00, 9, 214, 4.4, 10),
    (1003, 'Acer Swift Go 14', 'Laptop', (SELECT brand_id FROM brands WHERE name='Acer'), 44990.00, 6, 251, 4.1, 15),
    (1004, 'Acer Nitro V 15', 'Gaming Laptop', (SELECT brand_id FROM brands WHERE name='Acer'), 49990.00, 7, 288, 4.8, 20),
    (1005, 'Acer Predator Helios 16', 'Gaming Laptop', (SELECT brand_id FROM brands WHERE name='Acer'), 94990.00, 3, 325, 4.5, 0);


-- Handy queries (same ideas as the C++ menu)
-- Shop by brand:      SELECT p.* FROM products p JOIN brands b USING (brand_id) WHERE b.name = 'Apple';
-- Keyword search:     SELECT * FROM products WHERE name ILIKE '%iphone%';
-- Price range:        SELECT * FROM products WHERE price BETWEEN 10000 AND 20000 ORDER BY price;
-- Sort by stock:      SELECT * FROM products ORDER BY stock DESC;
-- Find by ID:         SELECT * FROM products WHERE product_id = 305;
