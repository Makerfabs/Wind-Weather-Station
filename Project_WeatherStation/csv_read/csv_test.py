import csv
import time

# data [] day0 []    weather0{'created_at', 'entry_id', 'humidity', 'drection', 'press', 'temperature', 'speed'}
#                    weather1
#                    ...

#         day1 []    weather0{'created_at', 'entry_id', 'humidity', 'drection', 'press', 'temperature', 'speed'}
#                    weather1
#                    ...
#         ...


def read_csv(data):
    with open('csv_read\weahter.csv', 'r') as f:
        reader = csv.reader(f)
        print(type(reader))
        result = list(reader)
        print(result[0])

        one_day = []
        day = 0

        for row in result[1:]:
            # print(row)
            # 判断日期变更
            date_time = time.strptime(row[0], '%Y/%m/%d %H:%M')
            if day != date_time.tm_yday:
                if day != 0:
                    data.append(one_day)
                one_day = []
                day = date_time.tm_yday

            weather = {}
            weather['time'] = date_time

            weather['index'] = int(row[1])
            weather['temp'] = float(row[5])
            weather['humi'] = int(row[2])
            weather['direct'] = int(row[3])
            weather['speed'] = float(row[6])
            weather['press'] = int(row[4])
            one_day.append(weather)

        print("Read Days:", end='\t')
        for day in data:
            print(day[0]['time'].tm_mday, end='\t')
        print()


def analyse_temp(data):
    temp_analysis = []
    for day in data:
        day_analysis = {}
        day_analysis['day'] = day[0]['time'].tm_mday
        day_analysis['count'] = 0
        day_analysis['avg_temp'] = 0
        day_analysis['max_temp'] = -100
        day_analysis['min_temp'] = 100

        for single in day:
            temp = single['temp']
            day_analysis['count'] = day_analysis['count'] + 1
            day_analysis['avg_temp'] = day_analysis['avg_temp'] + temp
            if temp > day_analysis['max_temp']:
                day_analysis['max_temp'] = temp
            if temp < day_analysis['min_temp']:
                day_analysis['min_temp'] = temp

        day_analysis['avg_temp'] = day_analysis['avg_temp'] / \
            day_analysis['count']

        temp_analysis.append(day_analysis)

    print("DAY      AVG     MAX     MIN")
    for day in temp_analysis:
        # print(i)
        print(day['day'], end='\t')
        print("%.2f" % day['avg_temp'], end='\t')
        print("%.2f" % day['max_temp'], end='\t')
        print("%.2f" % day['min_temp'])

    return temp_analysis


def analyse_humi(data):
    humi_analysis = []
    for day in data:
        day_analysis = {}
        day_analysis['day'] = day[0]['time'].tm_mday
        day_analysis['count'] = 0
        day_analysis['avg_humi'] = 0
        day_analysis['max_humi'] = -100
        day_analysis['min_humi'] = 100

        for single in day:
            temp = single['humi']
            day_analysis['count'] = day_analysis['count'] + 1
            day_analysis['avg_humi'] = day_analysis['avg_humi'] + temp
            if temp > day_analysis['max_humi']:
                day_analysis['max_humi'] = temp
            if temp < day_analysis['min_humi']:
                day_analysis['min_humi'] = temp

        day_analysis['avg_humi'] = day_analysis['avg_humi'] / \
            day_analysis['count']

        humi_analysis.append(day_analysis)

    print("DAY      AVG     MAX     MIN")
    for day in humi_analysis:
        # print(i)
        print(day['day'], end='\t')
        print("%.2f" % day['avg_humi'], end='\t')
        print("%.2f" % day['max_humi'], end='\t')
        print("%.2f" % day['min_humi'])

    return humi_analysis


def analyse_drect(data):
    drect_analysis = []
    for day in data:
        day_analysis = {}
        day_analysis['day'] = day[0]['time'].tm_mday
        day_analysis['count'] = 0
        day_analysis['avg_drect'] = 0

        for single in day:
            day_analysis['count'] = day_analysis['count'] + 1
            day_analysis['avg_drect'] = day_analysis['avg_drect'] + \
                single['direct']

        day_analysis['avg_drect'] = day_analysis['avg_drect'] / \
            day_analysis['count']

        drect_analysis.append(day_analysis)

    print("DAY      AVG")
    for day in drect_analysis:
        # print(i)
        print(day['day'], end='\t')
        print("%.2f" % day['avg_drect'])

    return drect_analysis


def analyse_speed(data):
    speed_analysis = []
    for day in data:
        day_analysis = {}
        day_analysis['day'] = day[0]['time'].tm_mday
        day_analysis['count'] = 0
        day_analysis['avg_speed'] = 0
        day_analysis['max_speed'] = -100
        day_analysis['min_speed'] = 100

        for single in day:
            temp = single['speed']
            day_analysis['count'] = day_analysis['count'] + 1
            day_analysis['avg_speed'] = day_analysis['avg_speed'] + temp
            if temp > day_analysis['max_speed']:
                day_analysis['max_speed'] = temp
            if temp < day_analysis['min_speed']:
                day_analysis['min_speed'] = temp

        day_analysis['avg_speed'] = day_analysis['avg_speed'] / \
            day_analysis['count']

        speed_analysis.append(day_analysis)

    print("DAY      AVG     MAX     MIN")
    for day in speed_analysis:
        # print(i)
        print(day['day'], end='\t')
        print("%.2f" % day['avg_speed'], end='\t')
        print("%.2f" % day['max_speed'], end='\t')
        print("%.2f" % day['min_speed'])
        
    return speed_analysis


def analyse_press(data):
    press_analysis = []
    for day in data:
        day_analysis = {}
        day_analysis['day'] = day[0]['time'].tm_mday
        day_analysis['count'] = 0
        day_analysis['avg_press'] = 0
        day_analysis['max_press'] = -100000
        day_analysis['min_press'] = 1000000

        for single in day:
            temp = single['press']
            day_analysis['count'] = day_analysis['count'] + 1
            day_analysis['avg_press'] = day_analysis['avg_press'] + temp
            if temp > day_analysis['max_press']:
                day_analysis['max_press'] = temp
            if temp < day_analysis['min_press']:
                day_analysis['min_press'] = temp

        day_analysis['avg_press'] = day_analysis['avg_press'] / \
            day_analysis['count']

        press_analysis.append(day_analysis)

    print("DAY      AVG     MAX     MIN")
    for day in press_analysis:
        # print(i)
        print(day['day'], end='\t')
        print("%.2f" % day['avg_press'], end='\t')
        print("%.2f" % day['max_press'], end='\t')
        print("%.2f" % day['min_press'])

    return press_analysis


def write_log():
    pass


weather_data = []
read_csv(weather_data)
print("Temperature")
analyse_temp(weather_data)
print("Humidity")
analyse_humi(weather_data)
print("Press")
analyse_press(weather_data)
print("Direct")
analyse_drect(weather_data)
print("Speed")
analyse_speed(weather_data)
