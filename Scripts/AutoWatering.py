"""AutoWatering.

Usage:
    AutoWatering create <deviceName> <numPairs> [--has_th_sensor] [--has_battery]

Options:
    -h, --help        show this help message and exit
    --has_th_sensor   if specified, it creates the feeds and dasboard blocks for temperature and humidity
    --has_battery     if specified, it creates the feeds and dashboard blocks for battery monitoring

"""

from docopt import docopt, DocoptExit
from Adafruit_IO_Python.Adafruit_IO import Client, RequestError, Feed, Group, Dashboard, Block
import os

aio = None

# Returns the specified group object
def get_group(group_name, createIfDoesntExist):
    try:
        # Try to get the group from Adafruit IO.
        print(f"Retrieving group {group_name}")
        group = aio.groups(group_name)
    except RequestError:
        # If the group does not exist, create it.
        if createIfDoesntExist:
            print(f"    Group doesn't exist. Creating it.")
            group = Group(name=group_name)
            group = aio.create_group(group)
        else:
            group = None
    return group

# Returns the specified feed object
def get_feed(feed_name, group_name, createIfDoesntExist):
    full_feed_name = f"{group_name}.{feed_name}"
    try:
        # Try to get the feed from Adafruit IO.
        print(f"Retrieving feed {feed_name}")
        feed = aio.feeds(full_feed_name)
    except RequestError:
        # If the feed does not exist, create it.
        if createIfDoesntExist:
            print(f"    Feed doesn't exist. Creating it.")
            feed = Feed(name=feed_name)
            feed = aio.create_feed(feed, group_name)
        else:
            feed = None
    return feed

def get_dashboard(dashboard_name, createIfDoesntExist):
    try:
        print(f"Retrieving dashboard {dashboard_name}")
        dashboard = aio.dashboards(dashboard_name)
    except RequestError:
        if createIfDoesntExist:
            print(f"    Dashboard doesn't exist. Creating it.")
            dashboard = Dashboard(name=dashboard_name)
            dashboard = aio.create_dashboard(dashboard)
        else:
            dashboard = None
    return dashboard


def get_feeds(feeds_names, group_name, createIfDoesntExist):
    res = {}
    for f in feeds_names:
        res[f] = get_feed(f, group_name, createIfDoesntExist)
    return res


def create_line_chart(dashboard_name, name, x, y, width, height, feeds, steppedLine = False):
    print(f"Creating block {name}. Type=line_chart")
    block_feeds = []
    for f in feeds:
        ele = block_feeds.append({})
        block_feeds[-1]['feed_id'] = f

    block = Block(
        name=name,
        id='',
        visual_type='line_chart',
        size_x=f"{width}", size_y=f"{height}", row=f"{y}", column=f"{x}",
        properties={
            "xAxisLabel": "X",
            "yAxisLabel": "Y",
            "yAxisMin": "",
            "yAxisMax": "",
            "decimalPlaces": "0",
            "rawDataOnly": False,
            "steppedLine": steppedLine,
            "feedKeyLegend": False,
            "gridLines": False,
            "historyHours": "24"
        },
        block_feeds = block_feeds
    )
    block = aio.create_block(dashboard_name, block)
    return block

def create_toggle_button(dashboard_name, name, x, y, width, height, feed, onValue = 1, offValue = 0):
    print(f"Creating block {name}. Type=toggle_button")
    block = Block(
        name=name,
        id='',
        visual_type='toggle_button',
        size_x=f"{width}", size_y=f"{height}", row=f"{y}", column=f"{x}",
        properties={
            "onText": "ON",
            "offText": "OFF",
            "onValue": f"{onValue}",
            "offValue": f"{offValue}",
        },
        block_feeds = [
            {
                "feed_id" : feed
            }
        ]
    )
    block = aio.create_block(dashboard_name, block)
    return block

def create_slider(dashboard_name, name, x, y, width, height, feed, minValue, maxValue, label, step = 1):
    print(f"Creating block {name}. Type=slider")
    block = Block(
        name=name,
        id='',
        visual_type='slider',
        size_x=f"{width}", size_y=f"{height}", row=f"{y}", column=f"{x}",
        properties={
            "min": f"{minValue}",
            "max": f"{maxValue}",
            "step": step,
            "label": label,
        },
        block_feeds = [
            {
                "feed_id" : feed
            }
        ]
    )
    block = aio.create_block(dashboard_name, block)
    return block

def create_momentary_button(dashboard_name, name, x, y, width, height, feed, pressValue, releaseValue, label, backgroundColor = '#90c83a'):
    print(f"Creating block {name}. Type=momentary_button")
    block = Block(
        name=name,
        id='',
        visual_type='momentary_button',
        size_x=f"{width}", size_y=f"{height}", row=f"{y}", column=f"{x}",
        properties={
            "text": label,
            "value": f"{pressValue}",
            "release": f"{releaseValue}",
            "backgroundColor": backgroundColor
        },
        block_feeds = [
            {
                "feed_id" : feed
            }
        ]
    )
    block = aio.create_block(dashboard_name, block)
    return block

def create_multiline_text(dashboard_name, name, x, y, width, height, feed):
    print(f"Creating block {name}. Type=multiline_text")
    block = Block(
        name=name,
        id='',
        visual_type='multiline_text',
        size_x=f"{width}", size_y=f"{height}", row=f"{y}", column=f"{x}",
        properties={
            "static": False,
            "fontSize": 12,
            "preformatted": False,
            "showIcon" : False,
            "decimalPlaces": "0"
        },
        block_feeds = [
            {
                "feed_id" : feed
            }
        ]
    )
    block = aio.create_block(dashboard_name, block)
    return block

def create_battery(dashboard_name, name, x, y, width, height, feed):
    print(f"Creating block {name}. Type=battery")
    block = Block(
        name=name,
        id='',
        visual_type='battery',
        size_x=f"{width}", size_y=f"{height}", row=f"{y}", column=f"{x}",
        properties={
            "highColor": "#477a00",
            "mediumColor": "#efab11",
            "lowColor": "#d42133",
            "lowConditions" : '[["<", "10.0"]]',
            "mediumConditions" : '[["<", "30.0"]]',
            "showNumbers": True
        },
        block_feeds = [
            {
                "feed_id" : feed
            }
        ]
    )
    block = aio.create_block(dashboard_name, block)
    return block

def create_device(deviceName, numPairs, hasTHSensor = False, hasBattery = False):

    if not deviceName.islower():
        print(f"Device name '{deviceName}' is invalid. A device name needs to be lowercase.")
        return

    print(f"Creating device {deviceName}, with {numPairs} sensor/motor pairs.")

    # Create the group if it doesn't exist
    group = get_group(deviceName, True)

    # Create the non-pair feeds
    allfeeds = get_feeds(
        {
            'devicename',
            'calibration-cancel',
            'calibration-index',
            'calibration-info',
            'calibration-reset',
            'calibration-save',
            'calibration-threshold',
         }
         , deviceName, True)

    if hasBattery:
        allfeeds.update(
            get_feeds(
                {
                    'battery-perc',
                    'battery-voltage',
                }
         , deviceName, True))

    if hasTHSensor:
        allfeeds.update(
            get_feeds(
                {
                    'humidity',
                    'temperature'
                }
         , deviceName, True))

    # Create the pair feeds
    for i in range(numPairs):
        pair_feeds = get_feeds(
            {
                f"group{i}-motoron",
                f"group{i}-running",
                f"group{i}-samplinginterval",
                f"group{i}-shotduration",
                f"group{i}-threshold",
                f"group{i}-value",
            },
            deviceName, True
        )
        allfeeds.update(pair_feeds)

    dashboard = get_dashboard(deviceName, True) 

    chart_width = 6
    chart_height = 4
    x = 0
    y = 0

    if hasBattery:
        create_battery(
            dashboard.name,
            '',
            x, y, 2, 1,
            allfeeds['battery-perc'].key
        )
        y += 1


    if hasTHSensor:
        create_line_chart(
            dashboard.name,
            'Temperature & humidity',
            x+2, y, chart_width, chart_height,
            {
                allfeeds['temperature'].key,
                allfeeds['humidity'].key
            }
            )
        y += chart_height
    
    chart_height = 5
    x = 0

    for i in range(numPairs):
        x = 0
        create_line_chart(
            dashboard.name,
            f"{i}-Sensor",
            x, y, chart_width, chart_height,
            {
                allfeeds[f"group{i}-motoron"].key,
                allfeeds[f"group{i}-value"].key,
            },
            steppedLine=True
        )

        x = chart_width

        create_toggle_button(
            dashboard.name,
            f"{i}-Running",
            x, y, 2, 1,
            allfeeds[f"group{i}-running"].key
        )
        create_toggle_button(
            dashboard.name,
            f"{i}-Shot",
            x+2, y, 2, 1,
            allfeeds[f"group{i}-motoron"].key,
            onValue = 100
        )

        create_slider(
            dashboard.name,
            f"{i}-Sampling interval",
            x, y+1, 4, 2,
            allfeeds[f"group{i}-samplinginterval"].key,
            0, 99,
            'minutes'
        )
        create_slider(
            dashboard.name,
            f"{i}-Water shot duration",
            x+4, y+1, 4, 2,
            allfeeds[f"group{i}-shotduration"].key,
            1, 99,
            'seconds'
        )
        create_slider(
            dashboard.name,
            f"{i}-Threshold",
            x, y+3, 4, 2,
            allfeeds[f"group{i}-threshold"].key,
            0, 100,
            '%'
        )
        create_momentary_button(
            dashboard.name,
            f"{i}-Calibrate",
            x+4, y+3, 4, 2,
            allfeeds['calibration-index'].key,
            i, -1,
            "Calibrate sensor"
        )
        y+= chart_height

    #
    # Calibration blocks
    #
    y += 2
    x = 0
    create_momentary_button(
        dashboard.name,
        '',
        x, y, 2, 2,
        allfeeds['calibration-reset'].key,
        1, 0, 'Start calibration',
        backgroundColor='#90c83a'
        )

    create_momentary_button(
        dashboard.name,
        '',
        x+2, y, 2, 2,
        allfeeds['calibration-save'].key,
        1, 0, 'Save calibration',
        backgroundColor = '#000bff'
        )

    create_momentary_button(
        dashboard.name,
        '',
        x+4, y, 2, 2,
        allfeeds['calibration-cancel'].key,
        1, 0, 'Cancel calibration',
        backgroundColor='#c83a3a'
        )

    create_slider(
        dashboard.name,
        'Threshold',
        x+6, y, 6, 2,
        allfeeds['calibration-threshold'].key,
        0, 100,
        '%'
    )

    create_multiline_text(
        dashboard.name,
        '',
        x, y+2, 6, 1,
        allfeeds['calibration-info'].key,
    )

    print("Finished creating device feeds and respective dashboard")

def run_tool(options):
    global aio
    aio_username = os.getenv('AIO_USER')
    aio_key = os.getenv('AIO_KEY')
    if (aio_username is None) or (aio_key is None):
        print("AIO_USER/AIO_KEY environment variables not set")
        exit(1)
        
    aio = Client(aio_username, aio_key)
    if options.get('create'):
        create_device(options.get('<deviceName>'), int(options.get('<numPairs>')), options.get('--has_th_sensor'), options.get('--has_battery'))

try:
    run_tool(docopt(__doc__))
except DocoptExit as e:
    print("Tool was invoked incorrectly. See usage below.")
    print(__doc__)
    exit(1)
except SystemExit:
    # SystemExit is raised if the user issues a -h|--help.
    pass
