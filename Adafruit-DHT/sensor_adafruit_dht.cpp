/*
 * Copyright (c) 2021-2022, RTduino Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-2-3       rose_man     first version
 */

#include <rtdevice.h>
#include <DHT.h>
#include <DHT_U.h>

#define DBG_TAG "sensor.adafruit.dht"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define DHTPIN 5     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

static rt_err_t adafruit_dht_init(DHT* dht_dev)
{
    dht_dev->begin();
    return RT_EOK;
}

static rt_ssize_t adafruit_dht_polling_get_data(rt_sensor_t sensor, rt_sensor_data_t data)
{
    float humidity, temp;
    DHT *dht_dev = (DHT *)sensor->parent.user_data;
    temp = dht_dev->readTemperature(false, true); /* populate temp and humidity objects with fresh data */
    humidity = dht_dev->readHumidity(true);
    if (sensor->info.type == RT_SENSOR_TYPE_TEMP)
    {
        data->data.temp = temp;
        data->timestamp = rt_sensor_get_ts();
        return 1;
    }
    else if (sensor->info.type == RT_SENSOR_TYPE_HUMI)
    {
        data->data.humi = humidity;
        data->timestamp = rt_sensor_get_ts();
        return 1;
    }
    else
    {
        return -RT_EINVAL;
    }
}

static rt_ssize_t adafruit_dht_fetch_data(rt_sensor_t sensor, rt_sensor_data_t buf, rt_size_t len)
{
    RT_ASSERT(buf);

    if (RT_SENSOR_MODE_GET_FETCH(sensor->info.mode) == RT_SENSOR_MODE_FETCH_POLLING)
    {
        return adafruit_dht_polling_get_data(sensor, buf);
    }
    else
    {
        return -RT_EINVAL;
    }
}

static rt_err_t adafruit_dht_control(rt_sensor_t sensor, int cmd, void *args)
{
    rt_err_t result = -RT_EINVAL;

    return result;
}

static struct rt_sensor_ops sensor_ops =
{
    adafruit_dht_fetch_data,
    adafruit_dht_control
};

static const char *sensor_name = "dht";

static int rt_hw_dht_init(void)
{
    rt_int8_t result;
    rt_sensor_t sensor_temp = RT_NULL, sensor_humi = RT_NULL;

    DHT *dht_dev = new DHT(DHTPIN, DHTTYPE);
    if (dht_dev == RT_NULL)
        return -1;

    if (adafruit_dht_init(dht_dev) != RT_EOK)
        return -1;

     /* temperature sensor register */
    sensor_temp = (rt_sensor_t)rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_temp == RT_NULL)
        return -1;

    sensor_temp->info.type       = RT_SENSOR_TYPE_TEMP;
    sensor_temp->info.vendor     = RT_SENSOR_VENDOR_ASAIR;
    sensor_temp->info.name       = sensor_name;
    sensor_temp->info.unit       = RT_SENSOR_UNIT_CELSIUS;
    sensor_temp->info.intf_type  = RT_SENSOR_INTF_ONEWIRE;

    sensor_temp->info.acquire_min = 1000;
    sensor_temp->info.accuracy.resolution = 0.01;
    sensor_temp->info.accuracy.error = 0.3;
    sensor_temp->info.scale.range_min = -40.0;
    sensor_temp->info.scale.range_max = 85.0;

    sensor_temp->config.intf.dev_name = RT_NULL;
    sensor_temp->config.intf.type = RT_SENSOR_INTF_ONEWIRE;
    sensor_temp->config.intf.arg = RT_NULL;
    sensor_temp->config.irq_pin.pin = PIN_IRQ_PIN_NONE;

    sensor_temp->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_temp, sensor_name, RT_DEVICE_FLAG_RDONLY, dht_dev);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    /* humidity sensor register */
    sensor_humi = (rt_sensor_t)rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_humi == RT_NULL)
    {
        return -1;
    }

    sensor_humi->info.type       = RT_SENSOR_TYPE_HUMI;
    sensor_humi->info.vendor     = RT_SENSOR_VENDOR_UNKNOWN;
    sensor_humi->info.name       = sensor_name;
    sensor_humi->info.unit       = RT_SENSOR_UNIT_PERCENTAGE;
    sensor_humi->info.intf_type  = RT_SENSOR_INTF_ONEWIRE;

    sensor_humi->info.acquire_min = 1000;
    sensor_humi->info.accuracy.resolution = 0.024;
    sensor_humi->info.accuracy.error = 2.0;
    sensor_humi->info.scale.range_min = 0.0;
    sensor_humi->info.scale.range_max = 100.0;

    sensor_humi->config.intf.dev_name = RT_NULL;
    sensor_humi->config.intf.type = RT_SENSOR_INTF_ONEWIRE;
    sensor_humi->config.intf.arg = RT_NULL;
    sensor_humi->config.irq_pin.pin = PIN_IRQ_PIN_NONE;

    sensor_humi->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_humi, sensor_name, RT_DEVICE_FLAG_RDONLY, dht_dev);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    return RT_EOK;

__exit:
    if (sensor_temp)
        rt_free(sensor_temp);
    if (sensor_humi)
        rt_free(sensor_humi);
    if (dht_dev)
        delete dht_dev;
    return -RT_ERROR;
}
INIT_ENV_EXPORT(rt_hw_dht_init);
