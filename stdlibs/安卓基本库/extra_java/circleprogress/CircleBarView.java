package rn_1;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.view.View;

public class CircleBarView extends View {
    public int size = 10;
    private Paint progressPaint;
    public int progress = 0;
    public int strokeWidth = 5;

    public CircleBarView(Context context) {
        super(context);
        init(context);
    }

    private void init(Context context) {
        this.progressPaint = new Paint();
        this.progressPaint.setStyle(Paint.Style.STROKE);
        this.progressPaint.setColor(-65536);
        this.progressPaint.setStrokeWidth(this.strokeWidth);
        this.progressPaint.setAntiAlias(true);
    }

    @Override
    protected void onMeasure(int i, int i2) {
        super.onMeasure(this.size + this.strokeWidth, this.size + this.strokeWidth);
        setMeasuredDimension(this.size + this.strokeWidth, this.size + this.strokeWidth);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        canvas.drawArc(new RectF(this.strokeWidth, this.strokeWidth, getMeasuredWidth() - this.strokeWidth,
                getMeasuredHeight() - this.strokeWidth), 270, 3.6f * this.progress, false, this.progressPaint);
        super.onDraw(canvas);
    }

    public void setProgress(int i) {
        this.progress = i;
        postInvalidate();
    }

    public void setViewSize(int i) {
        this.size = i;
        onMeasure(i, i);
        postInvalidate();
    }

    public void setStrokeWidth(int i) {
        Paint paint = this.progressPaint;
        this.strokeWidth = i;
        paint.setStrokeWidth(i);
        postInvalidate();
    }

    public void setColor(int i, int i2, int i3) {
        this.progressPaint.setColor(Color.rgb(i, i3, i2));
        postInvalidate();
    }

    public void setColor(int i) {
        this.progressPaint.setColor(i);
        postInvalidate();
    }
}