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
  `coord_x` float NOT NULL DEFAULT '0',
  `coord_y` float NOT NULL DEFAULT '0',
  `region` int NOT NULL,
  `nearest_point_interest` int DEFAULT NULL,
  `timestamp` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `FK__point_of_interest` (`nearest_point_interest`),
  CONSTRAINT `FK__point_of_interest` FOREIGN KEY (`nearest_point_interest`) REFERENCES `point_of_interest` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- Data exporting was unselected.

-- Dumping structure for table noise_level.point_of_interest
CREATE TABLE IF NOT EXISTS `point_of_interest` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(50) NOT NULL DEFAULT '',
  `region` int NOT NULL,
  `coord_x` float NOT NULL,
  `coord_y` float NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- Data exporting was unselected.

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IFNULL(@OLD_FOREIGN_KEY_CHECKS, 1) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40111 SET SQL_NOTES=IFNULL(@OLD_SQL_NOTES, 1) */;
