# Autowatering.py

This scripts allows creation of Adafruit IO group, feeds, and dashboard an AutoWatering device needs

# Adafruit_IO_Python

I opted to include a copy of Adafruit_IO_Python, due to some custom changes.
At the moment, the only change is in `Adafruit_IO\model.py`:

```
BLOCK_FIELDS  = [ 'name',
                  'id',
                  'visual_type',
                  'size_x', 'size_y','row','column',   << THIS IS NEW
                  'properties',
                  'block_feeds' ]
```

