/*
 * Copyright (c) 2021-2022, RTduino Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-11-19     Meco Man     first version
 */

#include <rtdevice.h>
#include <Adafruit_AHTX0.h>

#define DBG_TAG "sensor.adafruit.ahtx0"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static Adafruit_AHTX0 aht;

static rt_err_t adafruit_ahtx0_init(void)
{
    if (!aht.begin())
    {
        return -RT_ENOSYS;
    }
    return RT_EOK;
}

static rt_ssize_t adafruit_ahtx0_polling_get_data(rt_sensor_t sensor, rt_sensor_data_t data)
{
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp); /* populate temp and humidity objects with fresh data */

    if (sensor->info.type == RT_SENSOR_TYPE_TEMP)
    {
        data->data.temp = temp.temperature;
        data->timestamp = rt_sensor_get_ts();
        return 1;
    }
    else if (sensor->info.type == RT_SENSOR_TYPE_HUMI)
    {
        data->data.humi = humidity.relative_humidity;
        data->timestamp = rt_sensor_get_ts();
        return 1;
    }
    else
    {
        return -RT_EINVAL;
    }
}

static rt_ssize_t adafruit_ahtx0_fetch_data(rt_sensor_t sensor, rt_sensor_data_t buf, rt_size_t len)
{
    RT_ASSERT(buf);

    if (RT_SENSOR_MODE_GET_FETCH(sensor->info.mode) == RT_SENSOR_MODE_FETCH_POLLING)
    {
        return adafruit_ahtx0_polling_get_data(sensor, buf);
    }
    else
    {
        return -RT_EINVAL;
    }
}

static rt_err_t adafruit_ahtx0_control(rt_sensor_t sensor, int cmd, void *args)
{
    rt_err_t result = -RT_EINVAL;

    return result;
}

static struct rt_sensor_ops sensor_ops =
{
    adafruit_ahtx0_fetch_data,
    adafruit_ahtx0_control
};

static const char *sensor_name = "ahtx0";

static int rt_hw_ahtx0_init(void)
{
    rt_int8_t result;
    rt_sensor_t sensor_temp = RT_NULL, sensor_humi = RT_NULL;

    if (adafruit_ahtx0_init() != RT_EOK)
        return -1;

     /* temperature sensor register */
    sensor_temp = (rt_sensor_t)rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_temp == RT_NULL)
        return -1;

    sensor_temp->info.type       = RT_SENSOR_TYPE_TEMP;
    sensor_temp->info.vendor     = RT_SENSOR_VENDOR_ASAIR;
    sensor_temp->info.name       = sensor_name;
    sensor_temp->info.unit       = RT_SENSOR_UNIT_CELSIUS;
    sensor_temp->info.intf_type  = RT_SENSOR_INTF_I2C;

    sensor_temp->info.acquire_min = 1000;
    sensor_temp->info.accuracy.resolution = 0.01;
    sensor_temp->info.accuracy.error = 0.3;
    sensor_temp->info.scale.range_min = -40.0;
    sensor_temp->info.scale.range_max = 85.0;

    sensor_temp->config.intf.dev_name = Wire._i2c_bus_dev->parent.parent.name;
    sensor_temp->config.intf.type = RT_SENSOR_INTF_I2C;
    sensor_temp->config.intf.arg = RT_NULL;
    sensor_temp->config.irq_pin.pin = RT_PIN_NONE;

    sensor_temp->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_temp, sensor_name, RT_DEVICE_FLAG_RDONLY, RT_NULL);
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
    sensor_humi->info.vendor     = RT_SENSOR_VENDOR_ASAIR;
    sensor_humi->info.name       = sensor_name;
    sensor_humi->info.unit       = RT_SENSOR_UNIT_PERCENTAGE;
    sensor_humi->info.intf_type  = RT_SENSOR_INTF_I2C;

    sensor_humi->info.acquire_min = 1000;
    sensor_humi->info.accuracy.resolution = 0.024;
    sensor_humi->info.accuracy.error = 2.0;
    sensor_humi->info.scale.range_min = 0.0;
    sensor_humi->info.scale.range_max = 100.0;

    sensor_humi->config.intf.dev_name = Wire._i2c_bus_dev->parent.parent.name;
    sensor_humi->config.intf.type = RT_SENSOR_INTF_I2C;
    sensor_humi->config.intf.arg = RT_NULL;
    sensor_humi->config.irq_pin.pin = RT_PIN_NONE;

    sensor_humi->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_humi, sensor_name, RT_DEVICE_FLAG_RDONLY, RT_NULL);
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

    return -RT_ERROR;
}
INIT_ENV_EXPORT(rt_hw_ahtx0_init);
