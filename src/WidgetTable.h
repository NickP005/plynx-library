/**
 * @file       WidgetTable.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Sep 2016
 * @brief
 *
 */

#ifndef WidgetTable_h
#define WidgetTable_h

#include <Plynx/PlynxWidgetBase.h>

/**
 * @brief Fills the Table widget of the app with rows of a name and a value,
 * and hears when the person selects or moves a row.
 *
 * Each row has a number chosen by the sketch, a name and a value. The Table
 * keeps at most 100 rows and drops the oldest ones.
 *
 * Create it at the top level and connect it to the pin with
 * PLYNX_ATTACH_WIDGET(). Add rows where the data is.
 *
 * @code
 * WidgetTable table;
 * PLYNX_ATTACH_WIDGET(table, V8);
 *
 * int rows = 0;
 *
 * void logReading()
 * {
 *   table.addRow(rows, "Light", analogRead(A0));
 *   rows++;
 * }
 * @endcode
 *
 * @see addRow, PLYNX_ATTACH_WIDGET
 */
class WidgetTable
    : public PlynxWidgetBase
{
public:
    /** @brief A function the table calls when a row is selected or deselected. */
    typedef void (*ItemSelectChange)(int index, bool selected);
    /** @brief A function the table calls when a row is moved. */
    typedef void (*ItemOrderChange)(int indexFrom, int indexTo);

public:
    /**
     * @brief Creates the table object. Give the pin here, or later with
     * PLYNX_ATTACH_WIDGET().
     * @param vPin  The virtual pin of the Table widget.
     *
     * @code
     * WidgetTable table;
     * PLYNX_ATTACH_WIDGET(table, V8);
     * @endcode
     */
    WidgetTable(uint8_t vPin = -1)
        : PlynxWidgetBase(vPin)
        , mOnOrderChange(NULL)
        , mOnSelectChange(NULL)
    {}

    /**
     * @brief Handles what the Table widget writes, and calls the functions
     * given to onSelectChange() and onOrderChange().
     *
     * PLYNX_ATTACH_WIDGET() calls it. A sketch has no need to.
     *
     * @param request  The request, with the pin.
     * @param param    What the widget wrote.
     *
     * @code
     * WidgetTable table(V8);
     *
     * PLYNX_WRITE(V8)
     * {
     *   table.onWrite(request, param);   // what PLYNX_ATTACH_WIDGET does
     * }
     * @endcode
     */
    void onWrite(PlynxReq PLYNX_UNUSED &request, const PlynxParam& param) {
        if (mOnOrderChange && 0 == strcmp(param[0].asStr(), "order")) {
            mOnOrderChange(param[1].asInt(), param[2].asInt());
        } else if (mOnSelectChange && 0 == strcmp(param[0].asStr(), "select")) {
            mOnSelectChange(param[1].asInt(), true);
        } else if (mOnSelectChange && 0 == strcmp(param[0].asStr(), "deselect")) {
            mOnSelectChange(param[1].asInt(), false);
        }
    }

    /**
     * @brief Sets the function called when the person moves a row in the
     * Table.
     * @param cbk  A function `void name(int indexFrom, int indexTo)`.
     *
     * @code
     * WidgetTable table;
     * PLYNX_ATTACH_WIDGET(table, V8);
     *
     * void moved(int from, int to)
     * {
     *   Serial.print(from);
     *   Serial.print(" moved to ");
     *   Serial.println(to);
     * }
     *
     * void setup()
     * {
     *   table.onOrderChange(moved);
     * }
     * @endcode
     *
     * @see onSelectChange
     */
    void onOrderChange(ItemOrderChange cbk)   { mOnOrderChange = cbk; }

    /**
     * @brief Sets the function called when the person selects or deselects a
     * row in the Table.
     * @param cbk  A function `void name(int index, bool selected)`.
     *
     * @code
     * WidgetTable table;
     * PLYNX_ATTACH_WIDGET(table, V8);
     *
     * void chosen(int row, bool selected)
     * {
     *   digitalWrite(row + 2, selected ? HIGH : LOW);   // row 0 is pin 2
     * }
     *
     * void setup()
     * {
     *   table.onSelectChange(chosen);
     * }
     * @endcode
     *
     * @see onOrderChange, selectRow
     */
    void onSelectChange(ItemSelectChange cbk) { mOnSelectChange = cbk; }

    /**
     * @brief Removes all the rows from the Table.
     *
     * @code
     * WidgetTable table;
     * PLYNX_ATTACH_WIDGET(table, V8);
     *
     * PLYNX_CONNECTED()
     * {
     *   table.clear();
     * }
     * @endcode
     */
    void clear() {
        Plynx.virtualWrite(mPin, "clr");
    }

    /**
     * @brief Adds a row to the Table, or replaces the row with the same
     * number.
     *
     * @param index  The number of the row, chosen by the sketch.
     * @param name   The text on the left.
     * @param value  The text or number on the right.
     *
     * @code
     * WidgetTable table;
     * PLYNX_ATTACH_WIDGET(table, V8);
     *
     * void listRooms()
     * {
     *   table.addRow(0, "Kitchen", "21.5 C");
     *   table.addRow(1, "Bedroom", "19.0 C");
     * }
     * @endcode
     *
     * @warning The Table keeps 100 rows at most. Older rows go away.
     *
     * @see updateRow, clear
     */
    template <typename T1, typename T2>
    void addRow(int index, const T1& name, const T2& value) {
        Plynx.virtualWrite(mPin, "add", index, name, value);
    }

    /**
     * @brief Changes the name and the value of an existing row.
     *
     * @param index  The number of the row.
     * @param name   The new text on the left.
     * @param value  The new text or number on the right.
     *
     * @code
     * WidgetTable table;
     * PLYNX_ATTACH_WIDGET(table, V8);
     *
     * void refreshKitchen(float celsius)
     * {
     *   table.updateRow(0, "Kitchen", celsius);
     * }
     * @endcode
     *
     * @note A row number that does not exist changes nothing.
     *
     * @see addRow
     */
    template <typename T1, typename T2>
    void updateRow(int index, const T1& name, const T2& value) {
        Plynx.virtualWrite(mPin, "update", index, name, value);
    }

    /**
     * @brief Sends the command that asks the Table to scroll to a row.
     * @param index  The number of the row.
     *
     * @note The Plynx app (iOS, October 2026) receives the command and does
     * not scroll.
     *
     * @code
     * WidgetTable table;
     * PLYNX_ATTACH_WIDGET(table, V8);
     *
     * void showLast(int last)
     * {
     *   table.pickRow(last);
     * }
     * @endcode
     */
    void pickRow(int index) {
        Plynx.virtualWrite(mPin, "pick", index);
    }

    /**
     * @brief Marks a row as selected in the Table.
     * @param index  The number of the row.
     *
     * @code
     * WidgetTable table;
     * PLYNX_ATTACH_WIDGET(table, V8);
     *
     * void markDone(int row)
     * {
     *   table.selectRow(row);
     * }
     * @endcode
     *
     * @see deselectRow, onSelectChange
     */
    void selectRow(int index) {
        Plynx.virtualWrite(mPin, "select", index);
    }

    /**
     * @brief Removes the selection mark from a row in the Table.
     * @param index  The number of the row.
     *
     * @code
     * WidgetTable table;
     * PLYNX_ATTACH_WIDGET(table, V8);
     *
     * void markToDo(int row)
     * {
     *   table.deselectRow(row);
     * }
     * @endcode
     *
     * @see selectRow
     */
    void deselectRow(int index) {
        Plynx.virtualWrite(mPin, "deselect", index);
    }

private:
    ItemOrderChange  mOnOrderChange;
    ItemSelectChange mOnSelectChange;

};

#endif
