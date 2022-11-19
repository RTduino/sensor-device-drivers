#include <rtdevice.h>
#include <Adafruit_AHTX0.h>

#define DBG_TAG "sensor.adafruit.ahtx0"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define SENSOR_TEMP_RANGE_MAX (85)
#define SENSOR_TEMP_RANGE_MIN (-40)
#define SENSOR_HUMI_RANGE_MAX (100)
#define SENSOR_HUMI_RANGE_MIN (0)

static Adafruit_AHTX0 aht;

static rt_err_t _ahtx0_init(void)
{
    if (!aht.begin())
    {
        return -RT_ENOSYS;
    }
    return RT_EOK;
}

static rt_ssize_t _ahtx0_polling_get_data(rt_sensor_t sensor, rt_sensor_data_t data)
{
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp); /* populate temp and humidity objects with fresh data */

    if (sensor->info.type == RT_SENSOR_CLASS_TEMP)
    {
        data->data.temp = temp.temperature;
        data->timestamp = rt_sensor_get_ts();
        return 1;
    }
    else if (sensor->info.type == RT_SENSOR_CLASS_HUMI)
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


static rt_ssize_t ahtx0_fetch_data(rt_sensor_t sensor, rt_sensor_data_t buf, rt_size_t len)
{
    RT_ASSERT(buf);

    if (sensor->config.mode == RT_SENSOR_MODE_POLLING)
    {
        return _ahtx0_polling_get_data(sensor, buf);
    }
    else
    {
        return -RT_EINVAL;
    }
}


static rt_err_t ahtx0_control(rt_sensor_t sensor, int cmd, void *args)
{
    rt_err_t result = -RT_EINVAL;

    return result;
}

static struct rt_sensor_ops sensor_ops =
{
    ahtx0_fetch_data,
    ahtx0_control
};

static const char *sensor_name = "aht10/20";

int rt_hw_ahtx0_init(void)
{
    rt_int8_t result;
    rt_sensor_t sensor_temp = RT_NULL, sensor_humi = RT_NULL;


     /* temperature sensor register */
    sensor_temp = (rt_sensor_t)rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_temp == RT_NULL)
        return -1;

    sensor_temp->info.type       = RT_SENSOR_CLASS_TEMP;
    sensor_temp->info.vendor     = RT_SENSOR_VENDOR_ASAIR;
    sensor_temp->info.model      = sensor_name;
    sensor_temp->info.unit       = RT_SENSOR_UNIT_DCELSIUS;
    sensor_temp->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor_temp->info.range_max  = SENSOR_TEMP_RANGE_MAX;
    sensor_temp->info.range_min  = SENSOR_TEMP_RANGE_MIN;
    sensor_temp->info.period_min = 5;

    //rt_memcpy(&sensor_temp->config, cfg, sizeof(struct rt_sensor_config));
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

    sensor_humi->info.type       = RT_SENSOR_CLASS_HUMI;
    sensor_humi->info.vendor     = RT_SENSOR_VENDOR_ASAIR;
    sensor_humi->info.model      = sensor_name;
    sensor_humi->info.unit       = RT_SENSOR_UNIT_PERMILLAGE;
    sensor_humi->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor_humi->info.range_max  = SENSOR_HUMI_RANGE_MAX;
    sensor_humi->info.range_min  = SENSOR_HUMI_RANGE_MIN;
    sensor_humi->info.period_min = 5;

    //rt_memcpy(&sensor_humi->config, cfg, sizeof(struct rt_sensor_config));
    sensor_humi->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_humi, sensor_name, RT_DEVICE_FLAG_RDONLY, RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    _ahtx0_init();
    return RT_EOK;

__exit:
    if (sensor_temp)
        rt_free(sensor_temp);
    if (sensor_humi)
        rt_free(sensor_humi);

    return -RT_ERROR;
}
INIT_ENV_EXPORT(rt_hw_ahtx0_init);
