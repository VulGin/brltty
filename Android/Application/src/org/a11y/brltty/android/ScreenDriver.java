/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2013 by The BRLTTY Developers.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version. Please see the file LICENSE-GPL for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

package org.a11y.brltty.android;

import java.util.List;
import java.util.ArrayList;

import android.util.Log;

import android.graphics.Rect;

import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

public class ScreenDriver {
  private static final String LOG_TAG = ScreenDriver.class.getName();

  private static final String ROOT_NODE_NAME = "root";
  private static final boolean LOG_ACCESSIBILITY_EVENTS = true;

  private static BrailleRenderer brailleRenderer = new SimpleBrailleRenderer();

  private final static Object eventLock = new Object();
  private volatile static AccessibilityNodeInfo eventNode = null;

  private static AccessibilityNodeInfo currentNode = null;
  private static ScreenElementList screenElements;
  private static List<CharSequence> screenRows;
  private static int screenWidth;
  private static int cursorColumn;
  private static int cursorRow;

  private static AccessibilityNodeInfo getRootNode (AccessibilityNodeInfo node) {
    if (node != null) {
      while (true) {
        AccessibilityNodeInfo parent = node.getParent();

        if (parent == null) {
          break;
        }

        node = parent;
      }
    }

    return node;
  }

  private static String normalizeTextProperty (CharSequence text) {
    if (text != null) {
      String string = text.toString().trim();

      if (string.length() > 0) {
        return string;
      }
    }

    return null;
  }

  private static String getNodeText (AccessibilityNodeInfo node) {
    String text;

    if ((text = normalizeTextProperty(node.getContentDescription())) != null) {
      return text;
    }

    if ((text = normalizeTextProperty(node.getText())) != null) {
      return text;
    }

    if (node.getActions() != 0)  {
      return "";
    }

    return null;
  }

  private static void addPropertyValue (
    List<CharSequence> values,
    boolean condition,
    CharSequence value
  ) {
    if (condition) {
      values.add(value);
    }
  }

  private static void addNodeProperty (
    List<CharSequence> rows,
    CharSequence name,
    CharSequence ... values
  ) {
    if (values.length > 0) {
      StringBuilder sb = new StringBuilder();

      sb.append(name);
      sb.append(':');

      for (CharSequence value : values) {
        sb.append(' ');

        if (value == null) {
          sb.append("null");
        } else if (value.length() == 0) {
          sb.append("nil");
        } else {
          sb.append(value);
        }
      }

      rows.add(sb.toString());
    }
  }

  private static void addNodeProperties (
    List<CharSequence> rows,
    AccessibilityNodeInfo node
  ) {
    {
      List<CharSequence> values = new ArrayList<CharSequence>();
      addPropertyValue(values, node.isVisibleToUser(), "vis");
      addPropertyValue(values, node.getParent()==null, ROOT_NODE_NAME);

      {
        int count = node.getChildCount();
        addPropertyValue(values, count>0, "cld=" + count);
      }

      addPropertyValue(values, node.isEnabled(), "enb");
      addPropertyValue(values, node.isSelected(), "sel");
      addPropertyValue(values, node.isScrollable(), "scl");
      addPropertyValue(values, node.isFocusable(), "fcb");
      addPropertyValue(values, node.isFocused(), "fcd");
      addPropertyValue(values, node.isAccessibilityFocused(), "fca");
      addPropertyValue(values, node.isClickable(), "clk");
      addPropertyValue(values, node.isLongClickable(), "lng");
      addPropertyValue(values, node.isCheckable(), "ckb");
      addPropertyValue(values, node.isChecked(), "ckd");
      addPropertyValue(values, node.isPassword(), "pwd");
      addNodeProperty(rows, "flgs", values.toArray(new CharSequence[values.size()]));
    }

    addNodeProperty(rows, "desc", node.getContentDescription());
    addNodeProperty(rows, "text", node.getText());
    addNodeProperty(rows, "obj", node.getClassName());
    addNodeProperty(rows, "app", node.getPackageName());

    {
      AccessibilityNodeInfo subnode = node.getLabelFor();

      if (subnode != null) {
        addNodeProperty(rows, "lbl", getNodeText(subnode));
      }
    }

    {
      AccessibilityNodeInfo subnode = node.getLabeledBy();

      if (subnode != null) {
        addNodeProperty(rows, "lbd", getNodeText(subnode));
      }
    }

    {
      Rect rect = new Rect();
      node.getBoundsInScreen(rect);
      addNodeProperty(rows, "locn", rect.toShortString());
    }
  }

  private static void addSubtreeProperties (
    List<CharSequence> rows,
    AccessibilityNodeInfo root,
    CharSequence name
  ) {
    addNodeProperty(rows, "name", name);

    if (root != null) {
      addNodeProperties(rows, root);

      {
        int childCount = root.getChildCount();

        for (int childIndex=0; childIndex<childCount; childIndex+=1) {
          addSubtreeProperties(rows, root.getChild(childIndex), name + "." + childIndex);
        }
      }
    }
  }

  private static void logSubtreeProperties (AccessibilityNodeInfo root) {
    List<CharSequence> rows = new ArrayList<CharSequence>();
    addSubtreeProperties(rows, root, ROOT_NODE_NAME);

    for (CharSequence row : rows) {
      Log.d(LOG_TAG, row.toString());
    }
  }

  private static void logAccessibilityNode (AccessibilityNodeInfo node, String description) {
    StringBuilder sb = new StringBuilder();

    sb.append(description);
    sb.append(": ");

    if (node == null) {
      sb.append("null");
    } else {
      String text = getNodeText(node);

      if ((text != null) && (text.length() > 0)) {
        sb.append(text);
      } else {
        sb.append(node.getClassName());
      }

      Rect location = new Rect();
      node.getBoundsInScreen(location);
      sb.append(" ");
      sb.append(location.toShortString());
    }

    Log.d(LOG_TAG, sb.toString());
  }

  private static void logAccessibilityEvent (AccessibilityEvent event) {
    Log.d(LOG_TAG, "accessibility event: " + event.getEventType() + "(" + event.toString() + ")");

    AccessibilityNodeInfo node = event.getSource();
    logAccessibilityNode(node, "event node");
    logSubtreeProperties(getRootNode(node));
  }

  public static void handleAccessibilityEvent (AccessibilityEvent event) {
    if (LOG_ACCESSIBILITY_EVENTS) {
      logAccessibilityEvent(event);
    }

    switch (event.getEventType()) {
      case AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED:
      case AccessibilityEvent.TYPE_VIEW_FOCUSED:
      case AccessibilityEvent.TYPE_VIEW_ACCESSIBILITY_FOCUSED:
      case AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED:
        synchronized (eventLock) {
          eventNode = event.getSource();
        }
        break;

      default:
        break;
    }
  }

  private static void setScreenRows (List<CharSequence> rows) {
    if (rows == null) {
      rows = new ArrayList<CharSequence>();
    }

    if (rows.isEmpty()) {
      rows.add("empty screen");
    }

    int width = 1;
    for (CharSequence row : rows) {
      int length = row.length();

      if (length > width) {
        width = length;
      }
    }

    screenRows = rows;
    screenWidth = width;
  }

  private static void setCursorLocation (int column, int row) {
    cursorColumn = column;
    cursorRow = row;
  }

  private static void setCursorLocation () {
    AccessibilityNodeInfo node;

    {
      AccessibilityNodeInfo root = getRootNode(currentNode);

      if ((node = root.findFocus(AccessibilityNodeInfo.FOCUS_ACCESSIBILITY)) != null) {
        logAccessibilityNode(node, "accessibility focus");
      } else if ((node = root.findFocus(AccessibilityNodeInfo.FOCUS_INPUT)) != null) {
        logAccessibilityNode(node, "input focus");
      } else {
        node = currentNode;
        logAccessibilityNode(node, "event node");
      }
    }

    Rect visualLocation = new Rect();
    node.getBoundsInScreen(visualLocation);
    ScreenElement element = screenElements.findByVisualLocation(visualLocation.left, visualLocation.top);

    if (element != null) {
      Rect brailleLocation = element.getBrailleLocation();
      setCursorLocation(brailleLocation.left, brailleLocation.top);
    } else {
      setCursorLocation(0, 0);
    }
  }

  public static native void exportScreenProperties (
    int number,
    int columns, int rows,
    int column, int row
  );

  private static void renderSubtree (
    ScreenElementList elements,
    AccessibilityNodeInfo root
  ) {
    if (root != null) {
      if (root.isVisibleToUser()) {
        String text = getNodeText(root);

        if (text != null) {
          elements.add(text, root);
        }
      }

      {
        int childCount = root.getChildCount();

        for (int childIndex=0; childIndex<childCount; childIndex+=1) {
          renderSubtree(elements, root.getChild(childIndex));
        }
      }
    }
  }

  private static void renderSubtree (AccessibilityNodeInfo root) {
    List<CharSequence> rows = new ArrayList<CharSequence>();
    ScreenElementList elements = new ScreenElementList();

    renderSubtree(elements, root);
    brailleRenderer.renderElements(rows, elements);

    setScreenRows(rows);
    screenElements = elements;
    setCursorLocation();

    exportScreenProperties(
      root.getWindowId(),
      screenWidth, screenRows.size(),
      cursorColumn, cursorRow
    );
  }

  public static void updateCurrentView () {
    boolean hasChanged;

    synchronized (eventLock) {
      if ((hasChanged = (eventNode != null))) {
        currentNode = eventNode;
        eventNode = null;
      }
    }

    if (hasChanged) {
      renderSubtree(getRootNode(currentNode));
    }
  }

  public static void getRowText (char[] textBuffer, int rowIndex, int columnIndex) {
    CharSequence rowText = (rowIndex < screenRows.size())? screenRows.get(rowIndex): null;
    int rowLength = (rowText != null)? rowText.length(): 0;

    int textLength = textBuffer.length;
    int textIndex = 0;

    while (textIndex < textLength) {
      textBuffer[textIndex++] = (columnIndex < rowLength)? rowText.charAt(columnIndex++): ' ';
    }
  }

  public static boolean routeCursor (int column, int row) {
    if (row != -1) {
      ScreenElement element = screenElements.findByBrailleLocation(column, row);

      if (element != null) {
        if (element.performAction(column - element.getBrailleLocation().left)) {
          return true;
        }
      }
    }

    return false;
  }

  static {
    List<CharSequence> rows = new ArrayList<CharSequence>();
    rows.add("BRLTTY on Android");
    setScreenRows(rows);
  }
}
