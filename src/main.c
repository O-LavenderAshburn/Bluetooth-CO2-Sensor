#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(ess, LOG_LEVEL_INF);

#define LED_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

/* CO2 concentration in ppm */
static uint16_t co2_ppm;

static ssize_t read_co2(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t off)
{
	uint16_t value = sys_cpu_to_le16(co2_ppm);

	return bt_gatt_attr_read(conn, attr, buf, len, off, &value, sizeof(value));
}

static void ccc_changed(const struct bt_gatt_attr *attr, uint16_t value){
	LOG_INF("Notifications %s", value == BT_GATT_CCC_NOTIFY ? "on" : "off");
}

BT_GATT_SERVICE_DEFINE(ess_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),
	BT_GATT_CHARACTERISTIC(BT_UUID_GATT_CO2CONC,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ,
			       read_co2, NULL, NULL),
	BT_GATT_CCC(ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE));

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_ESS_VAL)),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
			sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static void start_adv(void)
{
	int err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad),
							  sd, ARRAY_SIZE(sd));
	if (err)
	{
		LOG_ERR("Advertising failed (%d)", err);
		return;
	}
	LOG_INF("Advertising");
}
static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err)
	{
		LOG_ERR("Connection failed (0x%02x)", err);
		return;
	}
	LOG_INF("Connected");
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected (0x%02x)", reason);
}

static void recycled(void)
{
	start_adv();
}

BT_CONN_CB_DEFINE(conn_cb) = {
	.connected = connected,
	.disconnected = disconnected,
	.recycled = recycled,
};

int main(void)
{
	int err;

	if (!gpio_is_ready_dt(&led))
	{
		LOG_ERR("LED not ready");
		return 0;
	}

	err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (err)
	{
		LOG_ERR("LED configure failed (%d)", err);
		return 0;
	}
	gpio_pin_set_dt(&led, 1);
	err = bt_enable(NULL);
	if (err)
	{
		LOG_ERR("bt_enable failed (%d)", err);
		return 0;
	}
	gpio_pin_set_dt(&led, 0);

	if (IS_ENABLED(CONFIG_SETTINGS))
	{
		settings_load();
	}

	start_adv();

	while (1)
	{
		co2_ppm = 400 + (k_uptime_get_32() / 1000) % 100;   /* fake for now */
		LOG_INF("CO2: %u ppm", co2_ppm);

		uint16_t value = sys_cpu_to_le16(co2_ppm);
		int nerr = bt_gatt_notify(NULL, &ess_svc.attrs[2], &value, sizeof(value));

		if (nerr && nerr != -ENOTCONN) {
			LOG_WRN("Notify failed (%d)", nerr);
		}

		k_msleep(1000);
	}

	return 0;
}