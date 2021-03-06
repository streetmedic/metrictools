/**
 * This file is part of the "FnordMetric" project
 *   Copyright (c) 2016 Laura Schlimmer, FnordCorp B.V.
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

function FnordMetricMetricDetail(elem, params) {
  'use strict';

  var config = new FnordMetricMetricDetailConfig(params);

  this.initialize = function() {
    initLayout();
    refreshSummary();
    refreshSeriesList();
  };

  this.destroy = function() {

  };

  function refresh(what_changed) {
    refreshSummary();
    refreshSeriesList();
  }

  function refreshSummary() {
    var chart_opts = {
      height: 120,
      axis_y_position: "inside",
      border_top: false,
      border_right: false,
      border_bottom: true,
      border_left: false,
      queries: [
        {
          query: {
            op: "fetch_summary",
            metric_id: config.getMetricID()
          }
        }
      ]
    };

    var chart = new FnordMetricChart(elem.querySelector(".summary"), chart_opts);
    chart.render();
  }

  function refreshSeriesList() {
    var widget_opts = {
      metric_id: config.getMetricID()
    };

    var widget = new FnordMetricTopSeries(elem.querySelector(".series_list"), widget_opts);
    widget.render();
  }

  function initLayout() {
    var page = templateUtil.getTemplate("fnordmetric-metric-detail-tpl");
    elem.appendChild(page);

    /** initialize ui components **/
    initHeaderBreadcrumbs();
    initLayoutDatepicker();
    initLayoutFilter();
  }

  function initHeaderBreadcrumbs() {
    elem.querySelector(".page_header .breadcrumbs .metric_name").innerHTML =
      DOMUtil.escapeHTML(config.getMetricID());
  }

  function initLayoutDatepicker() {
    var picker = new FM.TimeRangePickerComponent(
        elem.querySelector("f-timerange-picker"));

    picker.initialize({
      timezone: config.getTimezone(),
      from: config.getFrom(),
      until: config.getUntil()
    });

    picker.onSubmit(function(timerange) {
      refresh(timerange);
    });
  }

  function initLayoutFilter() {
    var filter = new FM.FilterComponent(elem.querySelector(".search input"));
    filter.init(config.getFilter());

    filter.onSubmit(function(filter_value) {
      refresh({filter: filter_value});
    });
  }

}

FnordMetric.views["fnordmetric.metric.detail"] = FnordMetricMetricDetail;
