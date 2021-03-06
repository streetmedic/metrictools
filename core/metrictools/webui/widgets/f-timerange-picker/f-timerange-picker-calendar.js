/**
 * This file is part of the "FnordMetric" project
 *   Copyright (c) 2016 Laura Schlimmer
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

var TimeRangePickerCalendar = function() {
  'use strict';

  var translations = {
    months: ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"],
    weekdays: ["M", "T", "W", "T", "F", "S", "S"]
  };

  var submit_callbacks = [];

  var elem;
  var date;
  var opts_;

  this.render = function(parent_elem, opts) {
    var tpl = templateUtil.getTemplate("f-timerange-picker-calendar-tpl");
    elem = tpl.querySelector("table");
    parent_elem.appendChild(tpl);

    opts_ = opts;

    if (opts_.selected) {
      date = new Date(opts_.selected);
    } else {
      date = new Date();
    }
    date.setDate("1");

    watchMonthMover();

    renderMonthTitle();
    renderWeekdayTitles();
    renderDates();
  }

  this.setTranslations = function(custom_translations) {
    translations = custom_translations;
  }

  this.setSubmitCallback = function(callback) {
    submit_callbacks.push(callback);
  }

/******************************** private *************************************/

  var watchMonthMover = function() {
    elem.querySelector("thead .prev").addEventListener("click", function(e) {
      date = getDateForMonthDiff(-1);
      update();
    }, false);

    elem.querySelector("thead .next").addEventListener("click", function(e) {
      date = getDateForMonthDiff(1);
      update();
    }, false);
  }

  var renderWeekdayTitles = function() {
    var tr = elem.querySelector("tr.weekdays");
    for (var i = 0; i < translations.weekdays.length; i++) {
      var td = document.createElement("td");
      td.innerHTML = translations.weekdays[i];
      tr.appendChild(td);
    }
  };

  var renderMonthTitle = function() {
    elem.querySelector(".title .month").innerHTML =
        translations.months[date.getMonth()];
    elem.querySelector(".title .year").innerHTML = date.getFullYear();
  }

  var renderDates = function() {
    var tr = elem.querySelector("tr.dates[data-week='0']");
    DOMUtil.clearChildren(tr);

    /** weekday of first day in selected month **/
    var counter = (date.getDay() + 6) % 7;
    renderDatesOfPrevMonth(tr, counter);

    /** render days of selected month **/
    var days_in_month = dateUtil.daysInMonth(
        date.getMonth() + 1,
        date.getFullYear());

    var all_disabled = false;

    var max_date = null;
    if (opts_.max) {

      if (opts_.max.getFullYear() < date.getFullYear() ||
          opts_.max.getFullYear() == date.getFullYear() &&
          opts_.max.getMonth() < date.getMonth()) {
        all_disabled = true;

      } else if (dateUtil.isSameMonth(date, opts_.max)) {
        max_date = opts_.max.getDate();

      }
    }

    var min_date = null;
    if (opts_.min) {

      if (opts_.min.getFullYear() > date.getFullYear() ||
          opts_.min.getFullYear() == date.getFullYear() &&
          opts_.min.getMonth() > date.getMonth()) {
        all_disabled = true;

      } else if (dateUtil.isSameMonth(date, opts_.min)) {
        min_date = opts_.min.getDate();

      }
    }

    var selected_date = null;
    if (opts_.selected && dateUtil.isSameMonth(date, opts_.selected)) {
      selected_date = opts_.selected.getDate();
    }

    var cur_date = null;
    if (dateUtil.isCurrentMonth(date)) {
      cur_date = new Date().getDate();
    }

    for (var i = 0; i < days_in_month; i++) {
      if (counter % 7 == 0) {
        tr = elem.querySelector("tr.dates[data-week='" + counter / 7 + "']");
        DOMUtil.clearChildren(tr);
      }

      var d = i + 1;

      var td = document.createElement("td");
      td.innerHTML = "<span>" + d + "</span>";

      /** highlight current date **/
      if (d == cur_date) {
        td.classList.add("cur_date");
      }

      /** highlight selected date **/
      if (d == selected_date) {
        td.classList.add("selected");
      }

      /** disable selection of dates > opts_.max **/
      if (all_disabled || min_date && d < min_date || max_date && d > max_date) {
        td.classList.add("disabled");

      } else {
        watchDate(td, d);
      }

      tr.appendChild(td);
      counter++;
    }

    renderDatesOfNextMonth(tr, counter);
  }

  var renderDatesOfPrevMonth = function(tr, start_offset) {
    var prev_month = getDateForMonthDiff(-1);
    var days_in_prev_month = dateUtil.daysInMonth(
        prev_month.getMonth() + 1,
        prev_month.getFullYear());


    /** render days of previous month **/
    for (var i = 0; i < start_offset; i++) {
      var td = document.createElement("td");
      td.className = "disabled";
      td.innerHTML = days_in_prev_month - start_offset + i + 1;
      tr.appendChild(td);
    }

    return 
  }

  var renderDatesOfNextMonth = function(tr, counter) {
    /** render days of next month for remaining cells **/
    for (var i = 0; counter / 7 != 6; i++) {
      if (counter % 7 == 0) {
        tr = elem.querySelector("tr.dates[data-week='" + counter / 7 + "']");
        DOMUtil.clearChildren(tr);
      }

      var td = document.createElement("td");
      td.className = "disabled";
      td.innerHTML = i + 1;
      tr.appendChild(td);
      counter++;
    }
  }

  //FIXME better naming
  var watchDate = function(td, date_num) {
    td.addEventListener("click", function(e) {
      date.setDate(date_num);
      submit();
    }, false);
  }

  //FIXME better naming
  var getDateForMonthDiff = function(diff) {
    var d = new Date(date);
    var month = (d.getMonth() + (diff)) % 12;
    d.setMonth(month);
    if (diff < 0 && month == 11) {
      d.setFullYear(d.getFullYear() - 1);
    }

    if (diff > 0 && month == 0) {
      d.setFullYear(d.getFullYear() + 1);
    }

    return d;
  }

  var update = function() {
    renderMonthTitle();
    renderDates();
  }

  var submit = function() {
    var d = new Date(date);
    submit_callbacks.forEach(function(callback) {
      callback(d);
    });
  }
}

