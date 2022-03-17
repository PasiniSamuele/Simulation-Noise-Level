import csv
import numpy as np

starting_x = 83
starting_y = 23
starting_noise = 33

min_x = 10
min_y = 10
max_x = 99
max_y = 99
min_noise = 10
max_noise = 99

max_delta_x = 10
max_delta_y = 10
max_delta_noise = 20

n_samples = 100
seed = 9

np.random.seed(seed)

actual_x = starting_x
actual_y = starting_y
actual_noise = starting_noise

with open('dataset.csv', 'w', encoding='UTF8', newline='') as f:
  writer = csv.writer(f, delimiter=',', quoting=csv.QUOTE_NONE, lineterminator='\n')
  for i in range(n_samples):
    delta_x = np.random.uniform(-max_delta_x/2, max_delta_x/2)
    delta_y = np.random.uniform(-max_delta_y/2, max_delta_y/2)
    delta_noise = np.random.randint(-max_delta_noise/2, max_delta_noise/2+1)

    if actual_x + delta_x > max_x or actual_x + delta_x < min_x:
      delta_x = - delta_x
    if actual_y + delta_y > max_y or actual_y + delta_y < min_y:
      delta_y = - delta_y
    if actual_noise + delta_noise > max_noise or actual_noise + delta_noise < min_noise:
      delta_noise = - delta_noise
    
    actual_x = round(actual_x + delta_x,2)
    actual_y = round(actual_y + delta_y,2)
    actual_noise = actual_noise + delta_noise
    str_x = format(actual_x,'.2f')
    str_y = format(actual_y,'.2f')
    row = [actual_noise, str_x, str_y]
    writer.writerow(row)
