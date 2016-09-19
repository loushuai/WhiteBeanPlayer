package com.whitebean.eventbus;

import com.squareup.otto.Bus;

import java.io.File;

/**
 * Created by loushuai on 2016/8/2.
 */
public class FileExplorerEvents {
    private static final Bus BUS = new Bus();

    public static Bus getBus() {
        return BUS;
    }

    private FileExplorerEvents() {
        // No instances.
    }

    public static class OnClickFile {
        public File mFile;

        public OnClickFile(String path) {
            this(new File(path));
        }

        public OnClickFile(File file) {
            mFile = file;
        }
    }
}
