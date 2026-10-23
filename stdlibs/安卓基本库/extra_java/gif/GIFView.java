package rn_1;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Movie;
import android.graphics.Paint;
import android.os.Build;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.view.View;
import java.io.InputStream;

public class GIFView extends View {
    private static final int DEFAULT_MOVIE_DURATION = 1000;
    private int mCurrentAnimationTime;
    private float mLeft;
    private int mMeasuredMovieHeight;
    private int mMeasuredMovieWidth;
    private Movie mMovie;
    private long mMovieStart;
    private volatile boolean mPaused;
    private float mScale;
    private float mTop;
    private boolean mVisible;
    private int moviemode;

    public GIFView(Context context) {
        this(context, null);
    }

    public GIFView(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
        this.mCurrentAnimationTime = 0;
        this.mVisible = true;
        this.mPaused = false;
        this.moviemode = 1;
        setViewAttributes(context, attributeSet);
    }

    private void drawMovieFrame(Canvas canvas) {
        this.mMovie.setTime(this.mCurrentAnimationTime);
        if (this.moviemode == 1) {
            canvas.save();
            canvas.scale(this.mScale, this.mScale);
            this.mMovie.draw(canvas, this.mLeft / this.mScale, this.mTop / this.mScale);
            canvas.restore();
            return;
        }
        this.mMovie.draw(canvas, 0.0f, 0.0f);
    }

    private void invalidateView() {
        if (!this.mVisible) {
            return;
        }
        if (Build.VERSION.SDK_INT >= 16) {
            postInvalidateOnAnimation();
        } else {
            invalidate();
        }
    }

    private void setViewAttributes(Context context, AttributeSet attributeSet) {
        if (Build.VERSION.SDK_INT >= 11) {
            setLayerType(1, null);
        }
        this.mPaused = false;
    }

    private void updateAnimationTime() {
        long uptimeMillis = SystemClock.uptimeMillis();
        if (this.mMovieStart == 0) {
            this.mMovieStart = uptimeMillis;
        }
        int duration = this.mMovie.duration();
        if (duration == 0) {
            duration = 1000;
        }
        this.mCurrentAnimationTime = (int) ((uptimeMillis - this.mMovieStart) % ((long) duration));
    }

    public Movie getMovie() {
        return this.mMovie;
    }

    public int getMovieMode() {
        return this.moviemode;
    }

    public int getMovieTime() {
        return this.mCurrentAnimationTime;
    }

    public boolean isPaused() {
        return this.mPaused;
    }

	@Override
    protected void onDraw(Canvas canvas) {
        if (this.mMovie == null) {
            return;
        }
        if (!this.mPaused) {
            updateAnimationTime();
            drawMovieFrame(canvas);
            invalidateView();
            return;
        }
        drawMovieFrame(canvas);
    }

	@Override
    protected void onLayout(boolean z, int i, int i2, int i3, int i4) {
        super.onLayout(z, i, i2, i3, i4);
        this.mLeft = ((float) (getWidth() - this.mMeasuredMovieWidth)) / 2.0f;
        this.mTop = ((float) (getHeight() - this.mMeasuredMovieHeight)) / 2.0f;
        this.mVisible = getVisibility() == 0;
    }

	@Override
    protected void onMeasure(int i, int i2) {
        if (this.mMovie != null) {
            int width = this.mMovie.width();
            int height = this.mMovie.height();
            int size = View.MeasureSpec.getSize(i);
            this.mScale = 1.0f / (((float) width) / ((float) size));
            this.mMeasuredMovieWidth = size;
            this.mMeasuredMovieHeight = (int) (((float) height) * this.mScale);
            if (1073741824 == View.MeasureSpec.getMode(i) || 1073741824 == View.MeasureSpec.getMode(i2)) {
                this.moviemode = 1;
                setMeasuredDimension(this.mMeasuredMovieWidth, this.mMeasuredMovieHeight);
                return;
            }
            this.moviemode = 2;
            setMeasuredDimension(width, height);
            return;
        }
        setMeasuredDimension(getSuggestedMinimumWidth(), getSuggestedMinimumHeight());
    }

    public void onScreenStateChanged(int i) {
        boolean z = true;
        super.onScreenStateChanged(i);
        if (i != 1) {
            z = false;
        }
        this.mVisible = z;
        invalidateView();
    }

    protected void onVisibilityChanged(View view, int i) {
        super.onVisibilityChanged(view, i);
        this.mVisible = i == 0;
        invalidateView();
    }

    protected void onWindowVisibilityChanged(int i) {
        super.onWindowVisibilityChanged(i);
        this.mVisible = i == 0;
        invalidateView();
    }

    public void setMovie(Movie movie) {
        this.mMovie = movie;
        requestLayout();
    }

    public void setMovieMode(int i) {
        this.moviemode = i;
        invalidateView();
    }

    public void setMovieResource(int i) {
        setMovieResource(getResources().openRawResource(i));
    }

    public void setMovieResource(InputStream inputStream) {
        this.mMovie = Movie.decodeStream(inputStream);
        requestLayout();
    }

    public void setMovieResource(String str) {
        this.mMovie = Movie.decodeFile(str);
        requestLayout();
    }

    public void setMovieResource(byte[] bArr) {
        this.mMovie = Movie.decodeByteArray(bArr, 0, bArr.length);
        requestLayout();
    }

    public void setMovieTime(int i) {
        this.mCurrentAnimationTime = i;
        invalidate();
    }

    public void setPaused(boolean z) {
        this.mPaused = z;
        if (!z) {
            this.mMovieStart = SystemClock.uptimeMillis() - ((long) this.mCurrentAnimationTime);
        }
        invalidate();
    }
}

