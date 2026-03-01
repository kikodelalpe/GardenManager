BEGIN TRANSACTION;
CREATE TABLE IF NOT EXISTS compatibilities(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	species_id INTEGER NOT NULL,
	product_id INTEGER NOT NULL,
	is_recommended INTEGER DEFAULT 1,
	notes TEXT,
	FOREIGN KEY(species_id) REFERENCES species(id),
	FOREIGN KEY(product_id) REFERENCES products(id)
);
CREATE TABLE IF NOT EXISTS event_event_types(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	event_id INTEGER NOT NULL,
	event_type_id INTEGER NOT NULL,
	FOREIGN KEY(event_id) REFERENCES events(id),
	FOREIGN KEY(event_type_id) REFERENCES event_types(id)
);
CREATE TABLE IF NOT EXISTS event_measurements(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	event_id INTEGER NOT NULL,
	metric_type TEXT,
	value REAL,
	FOREIGN KEY(event_id) REFERENCES events(id)
);
CREATE TABLE IF NOT EXISTS event_techniques(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	event_id INTEGER NOT NULL,
	technique_id INTEGER NOT NULL,
	FOREIGN KEY(event_id) REFERENCES events(id),
	FOREIGN KEY(technique_id) REFERENCES techniques(id)
);
CREATE TABLE IF NOT EXISTS event_types(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT NOT NULL,
	changes_to_status_id INTEGER,
	FOREIGN KEY(changes_to_status_id) REFERENCES plant_statuses(id)
);
CREATE TABLE IF NOT EXISTS event_usage(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	event_id INTEGER NOT NULL,
	batch_id INTEGER NOT NULL,
	quantity_used REAL,
	FOREIGN KEY(event_id) REFERENCES events(id),
	FOREIGN KEY(batch_id) REFERENCES inventory_batches(id)
);
CREATE TABLE IF NOT EXISTS events(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	timestamp TEXT NOT NULL,
	plant_id INTEGER,
	group_id INTEGER,
	notes TEXT,
	FOREIGN KEY(plant_id) REFERENCES plants(id),
	FOREIGN KEY(group_id) REFERENCES groups(id)
);
CREATE TABLE IF NOT EXISTS groups(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT NOT NULL,
	parent_id INTEGER,
	FOREIGN KEY(parent_id) REFERENCES groups(id)
);
CREATE TABLE IF NOT EXISTS inventory_batches(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	product_id INTEGER NOT NULL,
	purchase_date TEXT,
	lot_number TEXT,
	price_paid REAL,
	unit TEXT,
	quantity_initial REAL,
	quantity_current REAL,
	expiration_date TEXT,
	is_active INTEGER DEFAULT 1,
	FOREIGN KEY(product_id) REFERENCES products(id)
);
CREATE TABLE IF NOT EXISTS media_attachments(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	file_path TEXT NOT NULL,
	description TEXT,
	plant_id INTEGER,
	group_id INTEGER,
	species_id INTEGER,
	variety_id INTEGER,
	event_id INTEGER,
	product_id INTEGER,
	FOREIGN KEY(plant_id) REFERENCES plants(id) ON DELETE CASCADE,
	FOREIGN KEY(group_id) REFERENCES groups(id) ON DELETE CASCADE,
	FOREIGN KEY(species_id) REFERENCES species(id) ON DELETE CASCADE,
	FOREIGN KEY(variety_id) REFERENCES varieties(id) ON DELETE CASCADE,
	FOREIGN KEY(event_id) REFERENCES events(id) ON DELETE CASCADE,
	FOREIGN KEY(product_id) REFERENCES products(id) ON DELETE CASCADE,
	CONSTRAINT check_single_owner CHECK(
		(plant_id IS NOT NULL) +
		(group_id IS NOT NULL) +
		(species_id IS NOT NULL) +
		(variety_id IS NOT NULL) +
		(event_id IS NOT NULL) +
		(product_id IS NOT NULL) = 1
	)
);
CREATE TABLE IF NOT EXISTS plant_statuses(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT NOT NULL UNIQUE
);
CREATE TABLE IF NOT EXISTS plants(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	variety_id INTEGER NOT NULL,
	group_id INTEGER NOT NULL,
	seed_batch_id INTEGER,
	name TEXT,
	start_date TEXT,
	status_id INTEGER NOT NULL,
	FOREIGN KEY(variety_id) REFERENCES varieties(id),
	FOREIGN KEY(group_id) REFERENCES groups(id),
	FOREIGN KEY(seed_batch_id) REFERENCES inventory_batches(id),
	FOREIGN KEY(status_id) REFERENCES plant_statuses(id)
);
CREATE TABLE IF NOT EXISTS product_categories(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT UNIQUE NOT NULL
);
CREATE TABLE IF NOT EXISTS products(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT NOT NULL,
	manufacturer TEXT,
	category_id INTEGER NOT NULL,
	description TEXT,
	--fertilizer specific
	n_value REAL DEFAULT 0,
	p_value REAL DEFAULT 0,
	k_value REAL DEFAULT 0,
	is_organic INTEGER DEFAULT 0,
	--seed specific
	variety_id INTEGER,
	--soil specific
	ph REAL,
	recipe TEXT,
	FOREIGN KEY(variety_id) REFERENCES varieties(id),
	FOREIGN KEY(category_id) REFERENCES product_categories(id)
);
CREATE TABLE IF NOT EXISTS sensor_logs(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	timestamp TEXT NOT NULL,
	group_id INTEGER,
	sensor_type TEXT,
	value REAL,
	FOREIGN KEY(group_id) REFERENCES groups(id)
);
CREATE TABLE IF NOT EXISTS species(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	scientific_name TEXT NOT NULL,
	common_name TEXT NOT NULL,
	family TEXT NOT NULL,
	coltivation_guide TEXT,
	temp_min REAL,
	temp_max REAL,
	rh_min REAL,
	rh_max REAL,
	ph_soil_min REAL,
	ph_soil_max REAL,
	ph_water_min REAL,
	ph_water_max REAL
);
CREATE TABLE IF NOT EXISTS techniques(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT NOT NULL,
	icon_resource TEXT
);
CREATE TABLE IF NOT EXISTS units(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT NOT NULL,
	abbreviation TEXT NOT NULL,
	unit_type TEXT NOT NULL,
	system TEXT NOT NULL,
	multiplier_to_base REAL DEFAULT 1.0
);
CREATE TABLE IF NOT EXISTS varieties(
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	species_id INTEGER NOT NULL,
	name TEXT NOT NULL,
	days_to_maturity INTEGER,
	variety_description TEXT,
	FOREIGN KEY(species_id) REFERENCES species(id)
);
INSERT OR IGNORE INTO "event_types" ("id","name","changes_to_status_id") VALUES (1,'Plant',2);
INSERT OR IGNORE INTO "event_types" ("id","name","changes_to_status_id") VALUES (2,'Transplant',3);
INSERT OR IGNORE INTO "event_types" ("id","name","changes_to_status_id") VALUES (3,'Harvest',8);
INSERT OR IGNORE INTO "event_types" ("id","name","changes_to_status_id") VALUES (4,'Dead',9);
INSERT OR IGNORE INTO "event_types" ("id","name","changes_to_status_id") VALUES (5,'Note',NULL);
INSERT OR IGNORE INTO "event_types" ("id","name","changes_to_status_id") VALUES (6,'Fertilization',NULL);
INSERT OR IGNORE INTO "event_types" ("id","name","changes_to_status_id") VALUES (7,'Training',NULL);
INSERT OR IGNORE INTO "plant_statuses" ("id","name") VALUES (1,'Not started yet');
INSERT OR IGNORE INTO "plant_statuses" ("id","name") VALUES (2,'Planted');
INSERT OR IGNORE INTO "plant_statuses" ("id","name") VALUES (3,'Seedling');
INSERT OR IGNORE INTO "plant_statuses" ("id","name") VALUES (4,'Growing');
INSERT OR IGNORE INTO "plant_statuses" ("id","name") VALUES (5,'Flowering');
INSERT OR IGNORE INTO "plant_statuses" ("id","name") VALUES (6,'Fruiting');
INSERT OR IGNORE INTO "plant_statuses" ("id","name") VALUES (7,'Vegetative rest');
INSERT OR IGNORE INTO "plant_statuses" ("id","name") VALUES (8,'Harvested');
INSERT OR IGNORE INTO "plant_statuses" ("id","name") VALUES (9,'Dead');
INSERT OR IGNORE INTO "product_categories" ("id","name") VALUES (1,'Fertilizers');
INSERT OR IGNORE INTO "product_categories" ("id","name") VALUES (2,'Seeds');
INSERT OR IGNORE INTO "product_categories" ("id","name") VALUES (3,'Substrates');
INSERT OR IGNORE INTO "product_categories" ("id","name") VALUES (4,'Pots');
INSERT OR IGNORE INTO "product_categories" ("id","name") VALUES (5,'Tools');
INSERT OR IGNORE INTO "product_categories" ("id","name") VALUES (6,'Other');
INSERT OR IGNORE INTO "techniques" ("id","name","icon_resource") VALUES (1,'Pruning',NULL);
INSERT OR IGNORE INTO "techniques" ("id","name","icon_resource") VALUES (2,'Topping',NULL);
INSERT OR IGNORE INTO "techniques" ("id","name","icon_resource") VALUES (3,'Fimming',NULL);
INSERT OR IGNORE INTO "techniques" ("id","name","icon_resource") VALUES (4,'Lollipopping',NULL);
INSERT OR IGNORE INTO "techniques" ("id","name","icon_resource") VALUES (5,'Defoliation',NULL);
INSERT OR IGNORE INTO "techniques" ("id","name","icon_resource") VALUES (6,'Branches removing',NULL);
INSERT OR IGNORE INTO "techniques" ("id","name","icon_resource") VALUES (7,'LST',NULL);
INSERT OR IGNORE INTO "techniques" ("id","name","icon_resource") VALUES (8,'Mainlining',NULL);
INSERT OR IGNORE INTO "techniques" ("id","name","icon_resource") VALUES (9,'SOG',NULL);
INSERT OR IGNORE INTO "techniques" ("id","name","icon_resource") VALUES (10,'ScrOG',NULL);
INSERT OR IGNORE INTO "units" ("id","name","abbreviation","unit_type","system","multiplier_to_base") VALUES (1,'Pieces','pcs','count','universal', 1.0);
INSERT OR IGNORE INTO "units" ("id","name","abbreviation","unit_type","system","multiplier_to_base") VALUES (2,'Kilograms','kg','mass','metric', 1000.0);
INSERT OR IGNORE INTO "units" ("id","name","abbreviation","unit_type","system","multiplier_to_base") VALUES (3,'Grams','g','mass','metric', 1.0);
INSERT OR IGNORE INTO "units" ("id","name","abbreviation","unit_type","system","multiplier_to_base") VALUES (4,'Liters','l','volume','metric', 1000.0);
INSERT OR IGNORE INTO "units" ("id","name","abbreviation","unit_type","system","multiplier_to_base") VALUES (5,'Milliliters','ml','volume','metric', 1.0);
INSERT OR IGNORE INTO "units" ("id","name","abbreviation","unit_type","system","multiplier_to_base") VALUES (6,'Pounds','lbs','mass','imperial', 453.592);
INSERT OR IGNORE INTO "units" ("id","name","abbreviation","unit_type","system","multiplier_to_base") VALUES (7,'Ounces','oz','mass','imperial', 28.3495);
INSERT OR IGNORE INTO "units" ("id","name","abbreviation","unit_type","system","multiplier_to_base") VALUES (8,'Gallons','gal','volume','imperial', 3785.41);
COMMIT;
