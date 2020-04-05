package com.samsungnext.widget;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.util.AttributeSet;
import android.util.Pair;
import android.view.View;

public class OverlayView extends View {
    final private Paint paint = new Paint();
    private Point[] points;
    private Pair<Integer,Integer>[] bones;
    private int horizPadding = 0;

    public OverlayView(Context context, AttributeSet attrs) {
        super(context, attrs);
        points = new Point[0];
        paint.setColor(Color.BLUE);
        paint.setStrokeWidth(3);
    }

    public void drawPoints(Point[] points, int horizPadding) {
        this.points = points;
        this.horizPadding = horizPadding;
        invalidate();
    }

    public void setBones(Pair<Integer,Integer>[] bones) {
        this.bones = bones;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        for (int i = 0; i < points.length; i++) {
            //			Log.v(TAG, Float.toString(points[i].x) + "," + Float.toString(points[i].y));
            if (points[i].x > 0 && points[i].y > 0) {
                canvas.drawCircle(points[i].x + horizPadding, points[i].y, 10, paint);
            }
        }

        if (points.length > 0) {
            for (int i = 0; i < bones.length; i++) {
                final float x1 = (float) points[bones[i].first.intValue()].x;
                final float y1 = (float) points[bones[i].first.intValue()].y;
                final float x2 = (float) points[bones[i].second.intValue()].x;
                final float y2 = (float) points[bones[i].second.intValue()].y;

                if (x1 > 0 && y1 > 0 && x2 > 0 && y2 > 0) {
                    canvas.drawLine(x1 + horizPadding, y1, x2 + horizPadding, y2, paint);
                }
            }
        }
    }
}
