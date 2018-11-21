<?php

namespace Drupal\commerce_ups;

use Drupal\commerce_shipping\Entity\ShipmentInterface;
use Ups\Entity\Shipment;

/**
 * Interface to create and return a UPS API transit object.
 *
 * @package Drupal\commerce_ups
 */
interface UPSTransitRequestInterface {

  /**
   * Builds a time in transit object.
   *
   * @param array $configuration
   *   A configuration array from a CommerceShippingMethod.
   * @param \Drupal\commerce_shipping\Entity\ShipmentInterface $shipment
   *   The shipment.
   * @param \Ups\Entity\Shipment $api_shipment
   *   UPS Shipment Object.
   *
   * @return \Ups\Entity\TimeInTransitRequest
   *   A time in transit request response object for a shipment.
   */
  public function getTransitTime(
    array $configuration,
    ShipmentInterface $shipment,
    Shipment $api_shipment
  );

}
