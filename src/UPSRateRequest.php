<?php

namespace Drupal\commerce_ups;

use const COMMERCE_UPS_LOGGER_CHANNEL;
use Drupal\commerce_price\Price;
use Drupal\commerce_shipping\Entity\ShipmentInterface;
use Drupal\commerce_shipping\Plugin\Commerce\ShippingMethod\ShippingMethodInterface;
use Drupal\commerce_shipping\ShippingRate;
use Drupal\commerce_shipping\ShippingService;
use Drupal\Core\Datetime\DrupalDateTime;
use Drupal\Core\Logger\LoggerChannelFactoryInterface;
use Ups\Entity\RatedShipment;
use Ups\Entity\Shipment;
use Ups\Rate;
use Ups\Entity\RateInformation;

/**
 * Class UPSRateRequest.
 *
 * @package Drupal\commerce_ups
 */
class UPSRateRequest extends UPSRequest implements UPSRateRequestInterface {

  /**
   * The UPS Shipment object.
   *
   * @var \Drupal\commerce_ups\UPSShipmentInterface
   */
  protected $upsShipment;

  /**
   * The UPS transit request object.
   *
   * @var \Drupal\commerce_ups\UPSTransitRequestInterface
   */
  protected $upsTransit;

  /**
   * The logger.
   *
   * @var \Psr\Log\LoggerInterface
   */
  protected $logger;

  /**
   * UPSRateRequest constructor.
   *
   * @param \Drupal\commerce_ups\UPSShipmentInterface $ups_shipment
   *   The UPS shipment object.
   * @param \Drupal\commerce_ups\UPSTransitRequestInterface $ups_transit_request
   *   The UPS time transit service object.
   * @param \Drupal\Core\Logger\LoggerChannelFactoryInterface $logger_factory
   *   The logger factory.
   */
  public function __construct(
    UPSShipmentInterface $ups_shipment,
    UPSTransitRequestInterface $ups_transit_request,
    LoggerChannelFactoryInterface $logger_factory
  ) {
    $this->upsShipment = $ups_shipment;
    $this->upsTransit = $ups_transit_request;
    $this->logger = $logger_factory->get(COMMERCE_UPS_LOGGER_CHANNEL);
  }

  /**
   * Fetch rates from the UPS API.
   *
   * @param \Drupal\commerce_shipping\Entity\ShipmentInterface $commerce_shipment
   *   The commerce shipment.
   * @param \Drupal\commerce_shipping\Plugin\Commerce\ShippingMethod\ShippingMethodInterface $shipping_method
   *   The shipping method.
   *
   * @throws \Exception
   *   Exception when required properties are missing.
   *
   * @return array
   *   An array of ShippingRate objects.
   */
  public function getRates(ShipmentInterface $commerce_shipment, ShippingMethodInterface $shipping_method) {
    $rates = [];

    try {
      $auth = $this->getAuth();
    }
    catch (\Exception $e) {
      $this->logger->error(
        dt(
          'Unable to fetch authentication config for UPS. Please check your shipping method configuration.'
        )
      );

      return $rates;
    }

    $request = new Rate(
      $auth['access_key'],
      $auth['user_id'],
      $auth['password'],
      $this->useIntegrationMode()
    );

    try {
      $shipment = $this->upsShipment->getShipment($commerce_shipment, $shipping_method);

      // Enable negotiated rates, if enabled.
      if ($this->getRateType()) {
        $rate_information = new RateInformation();
        $rate_information->setNegotiatedRatesIndicator(TRUE);
        $rate_information->setRateChartIndicator(FALSE);
        $shipment->setRateInformation($rate_information);
      }

      // Shop Rates.
      $ups_rates = $request->shopRates($shipment);
    }
    catch (\Exception $e) {
      $this->logger->error($e->getMessage());
      $ups_rates = [];
    }

    if (empty($ups_rates->RatedShipment)) {
      return $rates;
    }

    foreach ($ups_rates->RatedShipment as $ups_rate) {
      $service_code = $ups_rate->Service->getCode();

      // Only add the rate if this service is enabled.
      if (!in_array($service_code, $this->configuration['services'])) {
        continue;
      }

      $rates[] = $this->getShippingRates($commerce_shipment, $shipment, $ups_rate);
    }

    return $rates;
  }

  /**
   * Gets the rate type: whether we will use negotiated rates or standard rates.
   *
   * @return bool
   *   Returns true if negotiated rates should be requested.
   */
  public function getRateType() {
    return boolval($this->configuration['rate_options']['rate_type']);
  }

  /**
   * Create and return a commerce shipping rate based on the UPS rate.
   *
   * @param \Drupal\commerce_shipping\Entity\ShipmentInterface $commerce_shipment
   *   The commerce shipment.
   * @param \Ups\Entity\Shipment $shipment
   *   The UPS shipment entity.
   * @param \Ups\Entity\RatedShipment $ups_rate
   *   The rate response from UPS.
   *
   * @return \Drupal\commerce_shipping\ShippingRate
   *   A commerce shipping rate object.
   */
  protected function getShippingRates(
    ShipmentInterface $commerce_shipment,
    Shipment $shipment,
    RatedShipment $ups_rate
  ) {
    $service_code = $ups_rate->Service->getCode();

    // Use negotiated rates if they were returned.
    if ($this->getRateType() && !empty($ups_rate->NegotiatedRates->NetSummaryCharges->GrandTotal->MonetaryValue)) {
      $cost = $ups_rate->NegotiatedRates->NetSummaryCharges->GrandTotal->MonetaryValue;
      $currency = $ups_rate->NegotiatedRates->NetSummaryCharges->GrandTotal->CurrencyCode;
    }
    // Otherwise, use the default rates.
    else {
      $cost = $ups_rate->TotalCharges->MonetaryValue;
      $currency = $ups_rate->TotalCharges->CurrencyCode;
    }

    $price = new Price((string) $cost, $currency);
    $service_name = $ups_rate->Service->getName();
    $date = new DrupalDateTime();
    $date->format('Y-m-d');

    $shipping_service = new ShippingService(
      $service_code,
      $service_name
    );

    $times = $this->upsTransit->getTransitTime(
      $this->configuration,
      $commerce_shipment,
      $shipment
    );

    foreach ($times->ServiceSummary as $serviceSummary) {
      return new ShippingRate(
        $service_code,
        $shipping_service,
        $price,
        $date::createFromFormat('Y-m-d', $serviceSummary->EstimatedArrival->getDate())
      );
    }
  }

}
