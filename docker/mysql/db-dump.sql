-- --------------------------------------------------------
-- Host:                         192.168.178.3
-- Server version:               8.0.28 - MySQL Community Server - GPL
-- Server OS:                    Linux
-- HeidiSQL Version:             11.3.0.6295
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;


-- Dumping database structure for noise_level
CREATE DATABASE IF NOT EXISTS `noise_level` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci */ /*!80016 DEFAULT ENCRYPTION='N' */;
USE `noise_level`;

-- Dumping structure for table noise_level.noise_measure
CREATE TABLE IF NOT EXISTS `noise_measure` (
  `id` int NOT NULL AUTO_INCREMENT,
  `noise` float NOT NULL DEFAULT '0',
  `region` int DEFAULT NULL,
  `coord_x` float NOT NULL DEFAULT '0',
  `coord_y` float NOT NULL DEFAULT '0',
  `nearest_point_interest` int DEFAULT NULL,
  `timestamp` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `FK2_noise_measure_point_of_interest` (`nearest_point_interest`),
  KEY `FK_noise_measure_region` (`region`),
  CONSTRAINT `FK2_noise_measure_point_of_interest` FOREIGN KEY (`nearest_point_interest`) REFERENCES `point_of_interest` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `FK_noise_measure_region` FOREIGN KEY (`region`) REFERENCES `region` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- Dumping data for table noise_level.noise_measure: ~4,623 rows (approximately)
DELETE FROM `noise_measure`;
/*!40000 ALTER TABLE `noise_measure` DISABLE KEYS */;
/*!40000 ALTER TABLE `noise_measure` ENABLE KEYS */;

-- Dumping structure for table noise_level.point_of_interest
CREATE TABLE IF NOT EXISTS `point_of_interest` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(50) NOT NULL DEFAULT '',
  `region` int NOT NULL,
  `coord_x` float NOT NULL,
  `coord_y` float NOT NULL,
  PRIMARY KEY (`id`),
  KEY `FK_point_of_interest_region` (`region`),
  CONSTRAINT `FK_point_of_interest_region` FOREIGN KEY (`region`) REFERENCES `region` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- Dumping data for table noise_level.point_of_interest: ~5 rows (approximately)
DELETE FROM `point_of_interest`;
/*!40000 ALTER TABLE `point_of_interest` DISABLE KEYS */;
INSERT INTO `point_of_interest` (`id`, `name`, `region`, `coord_x`, `coord_y`) VALUES
	(1, 'Colosseum', 1, 50, 50),
	(2, 'Pantheon ', 1, 75, 75),
	(3, 'Roman Forum', 1, 55, 55),
	(4, 'Trevi Fountain', 1, 20, 20),
	(5, 'St. Peter\'s Basilica', 1, 20, 80),
	(6, 'Milan Cathedral', 2, 50, 50),
	(7, ' Sforza Castle', 2, 75, 75),
	(8, 'Sempione Park', 2, 20, 20),
	(9, 'Museo del Novecento', 2, 80, 20),
	(10, 'La Scala', 2, 20, 70),
	(11, 'Uffizi Gallery', 3, 45, 45),
	(12, 'Florence Cathedral', 3, 50, 50),
	(13, 'Palazzo Vecchio', 3, 55, 55),
	(14, 'Ponte Vecchio', 3, 60, 60),
	(15, 'San Miniato al Monte', 3, 75, 75);
/*!40000 ALTER TABLE `point_of_interest` ENABLE KEYS */;

-- Dumping structure for table noise_level.region
CREATE TABLE IF NOT EXISTS `region` (
  `id` int NOT NULL,
  `min_x` int NOT NULL DEFAULT '0',
  `min_y` int NOT NULL DEFAULT '0',
  `max_x` int NOT NULL DEFAULT '0',
  `max_y` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- Dumping data for table noise_level.region: ~0 rows (approximately)
DELETE FROM `region`;
/*!40000 ALTER TABLE `region` DISABLE KEYS */;
INSERT INTO `region` (`id`, `min_x`, `min_y`, `max_x`, `max_y`) VALUES
	(1, 0, 0, 100, 100),
	(2, 0, 0, 100, 100),
	(3, 0, 0, 100, 100);
/*!40000 ALTER TABLE `region` ENABLE KEYS */;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IFNULL(@OLD_FOREIGN_KEY_CHECKS, 1) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40111 SET SQL_NOTES=IFNULL(@OLD_SQL_NOTES, 1) */;
